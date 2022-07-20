#include "nbpch.h"
#include "ScriptEngine.h"
#include "ScriptGlue.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>

namespace Nebula {
	namespace Utils {
		static char* ReadBytes(const std::filesystem::path& filepath, uint32_t* outSize)
		{
			std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
			NB_ASSERT(stream);

			std::streampos end = stream.tellg();
			stream.seekg(0, std::ios::beg);
			uint64_t size = end - stream.tellg();

			NB_ASSERT(size);

			char* buffer = new char[size];
			stream.read((char*)buffer, size);
			stream.close();

			*outSize = (uint32_t)size;
			return buffer;
		}

		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath)
		{
			uint32_t filesize = 0;
			char* fileData = ReadBytes(assemblyPath, &filesize);

			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData, filesize, 1, &status, 0);

			NB_ASSERT(status == MONO_IMAGE_OK, mono_image_strerror(status));

			std::string path = assemblyPath.string();
			MonoAssembly* assembly = mono_assembly_load_from_full(image, path.c_str(), &status, 0);
			mono_image_close(image);

			delete[] fileData;
			return assembly;
		}

		static void PrintAssemblyTypes(MonoAssembly* assembly)
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
	}

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		ScriptClass EntityClass;
	};

	static ScriptEngineData* s_Data = nullptr;

	void ScriptEngine::Init() 
	{
		s_Data = new ScriptEngineData();

		InitMono();
		LoadAssembly("Resources/Scripts/Nebula-ScriptCore.dll");

		ScriptGlue::RegisterFunctions();

		// Retrieve and Instanciate Class (With Constructor)
		s_Data->EntityClass = ScriptClass("Nebula", "Entity");
		MonoObject* instance = s_Data->EntityClass.Instanciate();

		// Call Method (No Params)
		MonoMethod* printMessageFunc = s_Data->EntityClass.GetMethod("PrintMessage", 0);
		s_Data->EntityClass.InvokeMethod(instance, printMessageFunc);
		
		// Call Method (2 Int Params)
		MonoMethod* printIntFunc = s_Data->EntityClass.GetMethod("PrintInts", 2);

		int value = 5;
		int value2 = 10;

		void* params[2] = {
			&value,
			&value2
		};

		s_Data->EntityClass.InvokeMethod(instance, printIntFunc, params);

		// Call Method (1 String Param)
		MonoMethod* printCustomMessageFunc = s_Data->EntityClass.GetMethod("PrintCustomMessage", 1);
		MonoString* string = mono_string_new(s_Data->AppDomain, "Hello World from C++");
		void* stringParam = string;
		s_Data->EntityClass.InvokeMethod(instance, printCustomMessageFunc, &stringParam);
	}

	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomain = mono_jit_init("NebulaJITRuntime");
		NB_ASSERT(rootDomain);
		
		s_Data->RootDomain = rootDomain;
	}

	void ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		s_Data->AppDomain = mono_domain_create_appdomain("NebulaScriptRuntime", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		s_Data->CoreAssembly = Utils::LoadMonoAssembly(filepath);
		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);

		// Utils::PrintAssemblyTypes(s_Data->CoreAssembly);
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

	MonoObject* ScriptEngine::InstanciateClass(MonoClass* monoClass) {
		MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
		mono_runtime_object_init(instance);
		return instance;
	}

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className)
		: m_ClassNamespace(classNamespace), m_ClassName(className)
	{
		m_MonoClass = mono_class_from_name(
			s_Data->CoreAssemblyImage, m_ClassNamespace.c_str(), m_ClassName.c_str());
	}

	MonoObject* ScriptClass::Instanciate() {
		return ScriptEngine::InstanciateClass(m_MonoClass);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameterCount) {
		return mono_class_get_method_from_name(m_MonoClass, name.c_str(), parameterCount);
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** parameters) {
		return mono_runtime_invoke(method, instance, parameters, nullptr);
	}
}