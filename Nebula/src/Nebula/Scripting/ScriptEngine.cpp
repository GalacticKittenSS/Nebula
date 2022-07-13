#include "nbpch.h"
#include "ScriptEngine.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>

namespace Nebula {

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
	};

	static ScriptEngineData* s_Data = nullptr;

	void ScriptEngine::Init() 
	{
		s_Data = new ScriptEngineData();

		InitMono();
	}

	char* ReadBytes(const std::string& filepath, uint32_t* outSize)
	{
		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
		NB_ASSERT(stream);

		std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		uint32_t size = end - stream.tellg();

		NB_ASSERT(size);

		char* buffer = new char[size];
		stream.read((char*)buffer, size);
		stream.close();

		*outSize = size;
		return buffer;
	}

	MonoAssembly* LoadCSharpAssembly(const std::string& assemblyPath)
	{
		uint32_t filesize = 0;
		char* fileData = ReadBytes(assemblyPath, &filesize);

		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(fileData, filesize, 1, &status, 0);

		NB_ASSERT(status == MONO_IMAGE_OK, mono_image_strerror(status));

		MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
		mono_image_close(image);

		delete[] fileData;
		return assembly;
	}

	void PrintAssemblyTypes(MonoAssembly* assembly)
	{
		MonoImage* image = mono_assembly_get_image(assembly);
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

			NB_TRACE("{0}.{1}", nameSpace, name);
		}
	}

	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomain = mono_jit_init("NebulaJITRuntime");
		NB_ASSERT(rootDomain);
		
		s_Data->RootDomain = rootDomain;

		s_Data->AppDomain = mono_domain_create_appdomain("NebulaScriptRuntime", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		s_Data->CoreAssembly = LoadCSharpAssembly("Resources/Scripts/Nebula-ScriptCore.dll");
		PrintAssemblyTypes(s_Data->CoreAssembly);

		MonoImage* assemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
		MonoClass* monoClass = mono_class_from_name(assemblyImage, "Nebula", "Main");

		MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
		mono_runtime_object_init(instance);

		// Call Method (No Params)
		MonoMethod* printMessageFunc = mono_class_get_method_from_name(monoClass, "PrintMessage", 0);
		mono_runtime_invoke(printMessageFunc, instance, nullptr, nullptr);
		
		// Call Method (2 Int Params)
		MonoMethod* printIntFunc = mono_class_get_method_from_name(monoClass, "PrintInts", 2);

		int value = 5;
		int value2 = 10;

		void* params[2] = {
			&value,
			&value2
		};

		mono_runtime_invoke(printIntFunc, instance, params, nullptr);

		// Call Method (1 String Param)
		MonoMethod* printCustomMessageFunc = mono_class_get_method_from_name(monoClass, "PrintCustomMessage", 1);
		MonoString* string = mono_string_new(s_Data->AppDomain, "Hello World from C++");
		void* stringParam = string;
		mono_runtime_invoke(printCustomMessageFunc, instance, &stringParam, nullptr);
	}

	void ScriptEngine::Shutdown() 
	{
		ShutdownMono();
		delete s_Data;
	}

	void ScriptEngine::ShutdownMono()
	{
		s_Data->AppDomain = nullptr;
		s_Data->RootDomain = nullptr;
	}
}