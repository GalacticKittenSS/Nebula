#include "nbpch.h"
#include "ScriptEngine.h"
#include "ScriptGlue.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/tabledefs.h>

namespace Nebula {
	static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap = {
		{ "System.Single", ScriptFieldType::Float },
		{ "System.Double", ScriptFieldType::Double },
		{ "System.Boolean", ScriptFieldType::Bool },
		{ "System.Char", ScriptFieldType::Char },
		{ "System.Int16", ScriptFieldType::Short },
		{ "System.Int32", ScriptFieldType::Int },
		{ "System.Int64", ScriptFieldType::Long },
		{ "System.Byte", ScriptFieldType::Byte },
		{ "System.SByte", ScriptFieldType::SByte },
		{ "System.UInt16", ScriptFieldType::UShort },
		{ "System.UInt32", ScriptFieldType::UInt },
		{ "System.UInt64", ScriptFieldType::ULong },
		{ "Nebula.Vector2", ScriptFieldType::Vector2 },
		{ "Nebula.Vector3", ScriptFieldType::Vector3 },
		{ "Nebula.Vector4", ScriptFieldType::Vector4 },
		{ "Nebula.Entity", ScriptFieldType::Entity }
	};

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

		ScriptFieldType MonoTypeToScriptFieldType(MonoType* type)
		{
			std::string name = mono_type_get_name(type);

			auto it = s_ScriptFieldTypeMap.find(name);
			if (it == s_ScriptFieldTypeMap.end())
				return ScriptFieldType::None;

			return it->second;
		}

		std::string ScriptFieldTypeToString(ScriptFieldType type)
		{
			switch (type)
			{
			case Nebula::ScriptFieldType::None: return "None";
			case Nebula::ScriptFieldType::Float: return "float";
			case Nebula::ScriptFieldType::Double: return "double";
			case Nebula::ScriptFieldType::Bool: return "bool";
			case Nebula::ScriptFieldType::Char: return "char";
			case Nebula::ScriptFieldType::Byte: return "byte";
			case Nebula::ScriptFieldType::Short: return "short";
			case Nebula::ScriptFieldType::Int: return "int";
			case Nebula::ScriptFieldType::Long: return "long";
			case Nebula::ScriptFieldType::SByte: return "sbyte";
			case Nebula::ScriptFieldType::UShort: return "ushort";
			case Nebula::ScriptFieldType::UInt: return "uint";
			case Nebula::ScriptFieldType::ULong: return "ulong";
			case Nebula::ScriptFieldType::Vector2: return "Vector2";
			case Nebula::ScriptFieldType::Vector3: return "Vector3";
			case Nebula::ScriptFieldType::Vector4: return "Vector4";
			case Nebula::ScriptFieldType::Entity: return "Entity";
			}

			NB_ASSERT(false);
			return "None";
		}

		std::string MonoTypeToString(MonoType* type)
		{
			ScriptFieldType field_type = MonoTypeToScriptFieldType(type);

			if (field_type == ScriptFieldType::None)
				return mono_type_get_name(type);

			return ScriptFieldTypeToString(field_type);
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

				const char* class_namespace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* class_name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

				NB_TRACE("{0}.{1}", class_namespace, class_name); 

				MonoClass* monoClass = mono_class_from_name(image, class_namespace, class_name);
				if (!monoClass)
					continue;

				int fieldcount = mono_class_num_fields(monoClass);
				NB_TRACE("{} has {} fields{}", class_name, fieldcount, fieldcount ? ":" : "");

				void* iterator = nullptr;
				while (MonoClassField* field = mono_class_get_fields(monoClass, &iterator))
				{
					const char* field_name = mono_field_get_name(field);

					uint32_t flags = mono_field_get_flags(field);
					bool field_public = flags & FIELD_ATTRIBUTE_PUBLIC;
						
					MonoType* field_type = mono_field_get_type(field);
					std::string field_typename = MonoTypeToString(field_type);

					NB_TRACE("	{} {} {}", field_public ? "public" : "private", field_typename, field_name);
				}
			}
		}
	}

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;

		ScriptClass EntityClass;
		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;
		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;

		// Runtime
		Scene* SceneContext = nullptr;
	};

	static ScriptEngineData* s_Data = nullptr;

	void ScriptEngine::Init() 
	{
		s_Data = new ScriptEngineData();

		InitMono();
		LoadAssembly("Resources/Scripts/Nebula-ScriptCore.dll");
		LoadAppAssembly("SandboxProj/Binaries/Sandbox.dll");

		s_Data->EntityClass = ScriptClass("Nebula", "Entity", true);
		LoadAssemblyClasses();

		ScriptGlue::RegisterFunctions();
		ScriptGlue::RegisterComponents();
		
#if 0
		// Retrieve and Instanciate Class (With Constructor)
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
#endif
	}

	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		s_Data->SceneContext = scene;
	}

	void ScriptEngine::OnRuntimeStop()
	{
		s_Data->SceneContext = nullptr;
		s_Data->EntityInstances.clear();
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		const auto& sc = entity.GetComponent<ScriptComponent>();
		if (!EntityClassExists(sc.ClassName))
			return;

		Ref<ScriptInstance> instance =
			CreateRef<ScriptInstance>(s_Data->EntityClasses[sc.ClassName], entity);

		UUID entityID = entity.GetUUID();
		s_Data->EntityInstances[entityID] = instance;

		if (s_Data->EntityScriptFields.find(entityID) != s_Data->EntityScriptFields.end())
		{
			const ScriptFieldMap& fieldMap = s_Data->EntityScriptFields.at(entityID);
			for (const auto& [name, fieldInstance] : fieldMap)
				instance->SetFieldValueInternal(name, fieldInstance.m_Buffer);
		}
				
		instance->InvokeOnCreate();
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, float ts)
	{
		bool instanceFound = s_Data->EntityInstances.find(entity.GetUUID()) != 
			s_Data->EntityInstances.end();

		if (!instanceFound)
			OnCreateEntity(entity);
		
		s_Data->EntityInstances[entity.GetUUID()]->InvokeOnUpdate(ts);
	}

	bool ScriptEngine::EntityClassExists(const std::string& signature)
	{
		return s_Data->EntityClasses.find(signature) != s_Data->EntityClasses.end();
	}

	Scene* ScriptEngine::GetSceneContext()
	{
		return s_Data->SceneContext;
	}

	const std::unordered_map<std::string, Ref<ScriptClass>> ScriptEngine::GetEntityClasses()
	{
		return s_Data->EntityClasses;
	}

	Ref<ScriptClass> ScriptEngine::GetEntityClass(std::string name)
	{
		auto it = s_Data->EntityClasses.find(name);
		if (it == s_Data->EntityClasses.end())
			return nullptr;

		return it->second;
	}
	
	ScriptFieldMap& ScriptEngine::GetScriptFieldMap(UUID id)
	{
		return s_Data->EntityScriptFields[id];
	}

	Ref<ScriptInstance> ScriptEngine::GetEntityScriptInstance(UUID id)
	{
		auto it = s_Data->EntityInstances.find(id);
		if (it == s_Data->EntityInstances.end())
			return nullptr;

		return it->second;
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage() {
		return s_Data->CoreAssemblyImage;
	}

	MonoDomain* ScriptEngine::GetAppDomain() {
		return s_Data->AppDomain;
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

		Utils::PrintAssemblyTypes(s_Data->CoreAssembly);
	}
	
	void ScriptEngine::LoadAppAssembly(const std::filesystem::path& filepath)
	{
		s_Data->AppAssembly = Utils::LoadMonoAssembly(filepath);
		s_Data->AppAssemblyImage = mono_assembly_get_image(s_Data->AppAssembly);

		Utils::PrintAssemblyTypes(s_Data->AppAssembly);
	}

	void ScriptEngine::LoadAssemblyClasses()
	{
		s_Data->EntityClasses.clear();

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_Data->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* class_namespace = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* class_name = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);
			
			MonoClass* monoClass = mono_class_from_name(s_Data->AppAssemblyImage, class_namespace, class_name);
			if (!monoClass)
				continue;

			bool isEntity = mono_class_is_subclass_of(monoClass, s_Data->EntityClass.GetMonoClass(), false);

			if (!isEntity)
				continue;

			std::string classSig = class_name;
			if (strlen(class_namespace) != 0)
				classSig = fmt::format("{}.{}", class_namespace, class_name);

			Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(class_namespace, class_name);
			s_Data->EntityClasses[classSig] = scriptClass;

			void* iterator = nullptr;
			while (MonoClassField* field = mono_class_get_fields(monoClass, &iterator))
			{
				const char* field_name = mono_field_get_name(field);
				
				uint32_t flags = mono_field_get_flags(field);
				if (!flags & FIELD_ATTRIBUTE_PUBLIC)
					continue;

				MonoType* mono_type = mono_field_get_type(field);
				ScriptFieldType field_type = Utils::MonoTypeToScriptFieldType(mono_type);

				scriptClass->m_Fields[field_name] = { field_type, field_name, field };
			}
		}
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

	MonoObject* ScriptEngine::InstanciateClass(MonoClass* monoClass) 
	{
		MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
		mono_runtime_object_init(instance);
		return instance;
	}

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore)
		: m_ClassNamespace(classNamespace), m_ClassName(className)
	{
		m_MonoClass = mono_class_from_name(
			isCore ? s_Data->CoreAssemblyImage : s_Data->AppAssemblyImage, m_ClassNamespace.c_str(), m_ClassName.c_str());
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

	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity) 
		: m_ScriptClass(scriptClass) 
	{
		m_Instance = m_ScriptClass->Instanciate();

		m_Constructor = s_Data->EntityClass.GetMethod(".ctor", 1);
		m_OnCreateMethod = scriptClass->GetMethod("OnCreate", 0);
		m_OnUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);

		UUID id = entity.GetUUID(); void* param = &id;
		m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);
	}

	void ScriptInstance::InvokeOnCreate() 
	{
		if (m_OnCreateMethod)
			m_ScriptClass->InvokeMethod(m_Instance, m_OnCreateMethod);
	}

	void ScriptInstance::InvokeOnUpdate(float ts) 
	{
		if (m_OnUpdateMethod)
		{
			void* param = &ts;
			m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateMethod, &param);
		}
	}

	bool ScriptInstance::GetFieldValueInternal(const std::string& name, void* buffer)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;

		mono_field_get_value(m_Instance, field.ClassField, buffer);
		return true;
	}
	 
	 bool ScriptInstance::SetFieldValueInternal(const std::string& name, const void* value)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;

		mono_field_set_value(m_Instance, field.ClassField, (void*)value);
		return true;
	}
}