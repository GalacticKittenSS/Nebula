#include "nbpch.h"
#include "ScriptEngine.h"
#include "ScriptGlue.h"

#include "Nebula/Core/Application.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/threads.h>

#include <FileWatch.hpp>

#include "Nebula/Utils/Time.h"
#include "Nebula/Core/FileSystem.h"
#include "Nebula/Project/Project.h"

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
		{ "Nebula.Entity", ScriptFieldType::Entity },
		{ "Nebula.Asset", ScriptFieldType::Asset },
		{ "Nebula.Prefab", ScriptFieldType::Prefab },
		{ "Nebula.Font", ScriptFieldType::Font },
		{ "Nebula.Texture", ScriptFieldType::Texture },
		{ "Nebula.Material", ScriptFieldType::Material },
	};

	namespace Utils {
		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath, bool loadPDB = false)
		{
			ScopedBuffer fileData = FileSystem::ReadFileBinary(assemblyPath);
			
			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData.As<char>(), (uint32_t)fileData.Size(), 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				NB_ERROR("[ScriptEngine] {}", errorMessage);
				return nullptr;
			}

			if (loadPDB)
			{
				std::filesystem::path pdbPath = assemblyPath;
				pdbPath.replace_extension(".pdb");

				if (std::filesystem::exists(pdbPath))
				{
					ScopedBuffer pdbFileData = FileSystem::ReadFileBinary(pdbPath);

					mono_debug_open_image_from_memory(image, pdbFileData.As<const mono_byte>(), (uint32_t)pdbFileData.Size());
					NB_INFO("Loaded PDB {}", pdbPath.string());
				}
			}

			std::string path = assemblyPath.string();
			MonoAssembly* assembly = mono_assembly_load_from_full(image, path.c_str(), &status, 0);
			mono_image_close(image);

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

		std::filesystem::path CoreAssemblyFilePath;
		std::filesystem::path AppAssemblyFilePath; 

		ScriptClass EntityClass;
		ScriptClass AssetClass;
		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityEditorInstances;
		
		Scope<filewatch::FileWatch<std::string>> AppAssemblyWatcher;
		bool AssemblyReloadPending = false;

#ifdef NB_DEBUG
		bool EnableDebugging = true;
#else
		bool EnableDebugging = false;
#endif
		// Runtime
		Scene* SceneContext = nullptr;
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityRuntimeInstances;

		Timer ReloadTimer;
	};

	static ScriptEngineData* s_Data = nullptr;

	static void OnAppAssemblyFileEvent(const std::string& path, const filewatch::Event change_type)
	{
		if (s_Data->AssemblyReloadPending || change_type != filewatch::Event::modified)
			return;

		s_Data->AssemblyReloadPending = true;
		s_Data->ReloadTimer = Timer();

		Application::Get().SubmitToMainThread([]() {
			s_Data->AppAssemblyWatcher.reset();
			ScriptEngine::ReloadAssembly();
		});
	}

	void ScriptEngine::Init()
	{
		s_Data = new ScriptEngineData();

		InitMono();
		ScriptGlue::RegisterFunctions();

		bool status = LoadAssembly("Resources/Scripts/Nebula-ScriptCore.dll");
		if (!status)
		{
			NB_WARN("[ScriptEngine] Could not load Core Assembly");
			return;
		}
		
		s_Data->EntityClass = ScriptClass("Nebula", "Entity", true);
		s_Data->AssetClass = ScriptClass("Nebula", "Asset", true);
		ScriptGlue::RegisterComponents();

		// Only load if active project
		status = Project::GetActive() && LoadAppAssembly(Project::GetActiveScriptModulePath());
		if (!status)
		{
			NB_WARN("[ScriptEngine] Could not load App Assembly");
			return;
		}

		LoadAssemblyClasses();
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();
		delete s_Data;
	}

	bool ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		s_Data->AppDomain = mono_domain_create_appdomain("NebulaScriptRuntime", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		s_Data->CoreAssembly = Utils::LoadMonoAssembly(filepath, s_Data->EnableDebugging);
		if (s_Data->CoreAssembly == nullptr)
			return false;

		s_Data->CoreAssemblyFilePath = filepath;
		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
		return true;
	}

	bool ScriptEngine::LoadAppAssembly(const std::filesystem::path& filepath)
	{
		s_Data->AppAssembly = Utils::LoadMonoAssembly(filepath, s_Data->EnableDebugging);
		if (s_Data->AppAssembly == nullptr)
			return false;

		s_Data->AppAssemblyFilePath = filepath;
		s_Data->AppAssemblyImage = mono_assembly_get_image(s_Data->AppAssembly);
		
		s_Data->AppAssemblyWatcher = CreateScope<filewatch::FileWatch<std::string>>(filepath.string(), OnAppAssemblyFileEvent);
		s_Data->AssemblyReloadPending = false;
		return true;
	}

	void ScriptEngine::ReloadAssembly()
	{
		fieldMap editor_field_values , runtime_field_values;
		signatureMap editor_class_sig, runtime_class_sig;
		
		GetScriptData(s_Data->EntityEditorInstances, editor_field_values, editor_class_sig);
		GetScriptData(s_Data->EntityRuntimeInstances, runtime_field_values, runtime_class_sig);
		
		mono_domain_set(mono_get_root_domain(), false);
		mono_domain_unload(s_Data->AppDomain);
		
		s_Data->EntityClasses.clear();
		
		bool status = LoadAssembly(s_Data->CoreAssemblyFilePath);
		if (!status)
		{
			NB_WARN("[ScriptEngine] Could not reload Core Assembly");

			s_Data->EntityEditorInstances.clear();
			s_Data->EntityRuntimeInstances.clear();

			return;
		}

		s_Data->EntityClass = ScriptClass("Nebula", "Entity", true);
		s_Data->AssetClass = ScriptClass("Nebula", "Asset", true);
		ScriptGlue::RegisterComponents();

		status = Project::GetActive() && LoadAppAssembly(Project::GetActiveScriptModulePath());
		if (!status)
		{
			NB_WARN("[ScriptEngine] Could not reload App Assembly");

			s_Data->EntityEditorInstances.clear();
			s_Data->EntityRuntimeInstances.clear();

			return;
		}
		
		LoadAssemblyClasses();

		ReloadScripts(s_Data->EntityEditorInstances, editor_field_values, editor_class_sig);
		if (s_Data->SceneContext)
		{
			s_Data->SceneContext->InitScripts();
			ReloadScripts(s_Data->EntityRuntimeInstances, runtime_field_values, runtime_class_sig);
		}
	}

	/* Runtime */

	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		s_Data->SceneContext = scene;
	}

	void ScriptEngine::OnRuntimeStop()
	{
		s_Data->SceneContext = nullptr;
		s_Data->EntityRuntimeInstances.clear();
	}

	bool ScriptEngine::CreateRuntimeScript(Entity entity)
	{
		const auto& sc = entity.GetComponent<ScriptComponent>();
		if (!EntityClassExists(sc.ClassName))
			return false;

		UUID entityID = entity.GetUUID();
		Ref<ScriptInstance> editorInstance = nullptr;

		auto it = s_Data->EntityEditorInstances.find(entityID);
		if (it != s_Data->EntityEditorInstances.end())
			editorInstance = it->second;

		Ref<ScriptInstance> runtimeInstance = editorInstance ? 
			CreateRef<ScriptInstance>(editorInstance, entity) :
			CreateRef<ScriptInstance>(s_Data->EntityClasses[sc.ClassName], entity);

		s_Data->EntityRuntimeInstances[entityID] = runtimeInstance;
		return true;
	}

	bool ScriptEngine::OnCreateEntity(Entity entity)
	{
		bool instanceFound = s_Data->EntityRuntimeInstances.find(entity.GetUUID()) !=
			s_Data->EntityRuntimeInstances.end();

		if (!instanceFound)
		{
			if (!CreateRuntimeScript(entity))
				return false;
		}

		s_Data->EntityRuntimeInstances[entity.GetUUID()]->InvokeOnCreate();
		return true;
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, float ts)
	{
		auto it = s_Data->EntityRuntimeInstances.find(entity.GetUUID());
		bool instanceFound = it != s_Data->EntityRuntimeInstances.end();
		
		if (!instanceFound || !it->second->OnCreateCalled())
		{
			if (!OnCreateEntity(entity))
				return;
		}
		
		s_Data->EntityRuntimeInstances[entity.GetUUID()]->InvokeOnUpdate(ts);
	}
	
	void ScriptEngine::OnCollisionEnter(Entity entity, Entity other)
	{
		if (!s_Data->SceneContext)
			return;

		bool instanceFound = s_Data->EntityRuntimeInstances.find(entity.GetUUID()) != 
			s_Data->EntityRuntimeInstances.end();

		if (!instanceFound)
		{
			if (!OnCreateEntity(entity))
				return;
		}
		
		s_Data->EntityRuntimeInstances[entity.GetUUID()]->InvokeOnCollisionEnter(other);
	}
	
	void ScriptEngine::OnCollisionExit(Entity entity, Entity other)
	{
		if (!s_Data->SceneContext)
			return;

		bool instanceFound = s_Data->EntityRuntimeInstances.find(entity.GetUUID()) != 
			s_Data->EntityRuntimeInstances.end();

		if (!instanceFound)
		{
			if (!OnCreateEntity(entity))
				return;
		}
		
		s_Data->EntityRuntimeInstances[entity.GetUUID()]->InvokeOnCollisionExit(other);
	}

	void ScriptEngine::DeleteScriptInstance(UUID entityID)
	{
		if (s_Data->SceneContext)
			s_Data->EntityRuntimeInstances.erase(entityID);
		else
			s_Data->EntityEditorInstances.erase(entityID);
	}

	void ScriptEngine::CopyScriptFields(Entity from, Entity to)
	{
		Ref<ScriptInstance> from_instance = GetScriptInstance(from);
		Ref<ScriptInstance> to_instance = GetScriptInstance(to);

		if (from_instance->GetScriptClass() != to_instance->GetScriptClass())
			return;

		const auto& fields = from_instance->GetScriptClass()->GetFields();
		for (const auto& [name, field] : fields)
		{
			static char buffer[16];

			switch (field.Type)
			{
			case ScriptFieldType::None: break;
			case ScriptFieldType::Prefab:
			case ScriptFieldType::Font:
			case ScriptFieldType::Texture:
			case ScriptFieldType::Asset:
			{
				from_instance->GetFieldValueInternal(field.Name, buffer);
				uint64_t id = GetIDFromObject(*(MonoObject**)buffer);
				MonoObject* object = CreateAssetClass(id);
				to_instance->SetFieldValueInternal(field.Name, object);
				break;
			}
			case ScriptFieldType::Entity:
			{
				from_instance->GetFieldValueInternal(field.Name, buffer);
				uint64_t id = GetIDFromObject(*(MonoObject**)buffer);
				MonoObject* object = CreateEntityClass(id);
				to_instance->SetFieldValueInternal(field.Name, object);
				break;
			}
			default:
				from_instance->GetFieldValueInternal(field.Name, buffer);
				to_instance->SetFieldValueInternal(field.Name, buffer);
				break;
			}
		}
	}

	/* ------- */

	void ScriptEngine::ClearScriptInstances()
	{
		s_Data->EntityEditorInstances.clear();
		s_Data->EntityRuntimeInstances.clear();
	}

	Ref<ScriptInstance> ScriptEngine::CreateScriptInstance(Entity entity)
	{
		UUID entityID = entity.GetUUID();

		const auto& sc = entity.GetComponent<ScriptComponent>();
		if (!EntityClassExists(sc.ClassName))
		{
			DeleteScriptInstance(entityID);
			return nullptr;
		}
			
		if (s_Data->SceneContext)
		{
			CreateRuntimeScript(entity);
			return s_Data->EntityRuntimeInstances.at(entityID);
		}
		
		Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_Data->EntityClasses[sc.ClassName], entity);
		s_Data->EntityEditorInstances[entityID] = instance;
		
		return instance;
	}

	Ref<ScriptInstance> ScriptEngine::GetScriptInstance(Entity entity)
	{
		const auto& instances = s_Data->SceneContext ? s_Data->EntityRuntimeInstances : s_Data->EntityEditorInstances;

		auto it = instances.find(entity.GetUUID());
		if (it == instances.end())
			return CreateScriptInstance(entity);

		return it->second;
	}

	MonoObject* ScriptEngine::CreateEntityClass(UUID entityID)
	{
		MonoObject* instance = s_Data->EntityClass.Instanciate();
		MonoMethod* constructor = s_Data->EntityClass.GetMethod(".ctor", 1);

		void* param = &entityID;
		s_Data->EntityClass.InvokeMethod(instance, constructor, &param);
		return instance;
	}

	MonoObject* ScriptEngine::CreateAssetClass(AssetHandle handle)
	{
		MonoObject* instance = s_Data->AssetClass.Instanciate();
		MonoMethod* constructor = s_Data->AssetClass.GetMethod(".ctor", 1);

		void* param = &handle;
		s_Data->AssetClass.InvokeMethod(instance, constructor, &param);
		return instance;
	}

	MonoString* ScriptEngine::CreateMonoString(const char* string) {
		return mono_string_new(s_Data->AppDomain, string);
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
	
	bool ScriptEngine::EntityClassExists(const std::string& signature)
	{
		return s_Data->EntityClasses.find(signature) != s_Data->EntityClasses.end();
	}

	MonoObject* ScriptEngine::GetManagedInstance(UUID uuid)
	{
		auto it = s_Data->EntityRuntimeInstances.find(uuid);
		if (it == s_Data->EntityRuntimeInstances.end())
			return nullptr;

		return it->second->GetManagedObject();
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage() {
		return s_Data->CoreAssemblyImage;
	}

	uint64_t ScriptEngine::GetIDFromObject(MonoObject* object)
	{
		if (!object)
			return NULL;

		bool isEntity = mono_object_isinst(object, s_Data->EntityClass.GetMonoClass());
		bool isAsset = mono_object_isinst(object, s_Data->AssetClass.GetMonoClass());

		if (!isEntity && !isAsset)
			return NULL;

		MonoClassField* field = nullptr;

		if (isEntity)
			field = mono_class_get_field_from_name(s_Data->EntityClass.GetMonoClass(), "ID");

		if (isAsset)
			field = mono_class_get_field_from_name(s_Data->AssetClass.GetMonoClass(), "AssetHandle");

		NB_ASSERT(field);
		
		char buffer[16];
		mono_field_get_value(object, field, buffer);
		return *(uint64_t*)buffer;
	}

	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");

		if (s_Data->EnableDebugging)
		{
			const char* argv[2] = {
				"--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,loglevel=3,logfile=MonoDebugger.log",
				"--soft-breakpoints"
			};

			mono_jit_parse_options(2, (char**)argv);
			mono_debug_init(MONO_DEBUG_FORMAT_MONO);
		}

		MonoDomain* rootDomain = mono_jit_init("NebulaJITRuntime");
		NB_ASSERT(rootDomain);
		s_Data->RootDomain = rootDomain;

		if (s_Data->EnableDebugging)
			mono_debug_domain_create(s_Data->RootDomain);

		mono_thread_set_main(mono_thread_current());
	}

	void ScriptEngine::ShutdownMono()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_Data->AppDomain);
		s_Data->AppDomain = nullptr;

		mono_jit_cleanup(s_Data->RootDomain);
		s_Data->RootDomain = nullptr;
	}

	MonoObject* ScriptEngine::InstanciateClass(MonoClass* monoClass)
	{
		MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
		mono_runtime_object_init(instance);
		return instance;
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
				bool field_public = flags & FIELD_ATTRIBUTE_PUBLIC,
					field_static = flags & FIELD_ATTRIBUTE_STATIC;

				if (!field_public || field_static)
					continue;

				MonoType* mono_type = mono_field_get_type(field);
				ScriptFieldType field_type = Utils::MonoTypeToScriptFieldType(mono_type);

				scriptClass->m_Fields[field_name] = { field_type, field_name, field };
			}
		}
	}

	void ScriptEngine::ReloadScripts(std::unordered_map<UUID, Ref<ScriptInstance>>& instances,
		fieldMap& field_values, signatureMap& classSig)
	{
		Array<UUID> instancesToRemove;
		for (auto& [entityID, instance] : instances)
		{
			if (instances == s_Data->EntityEditorInstances)
			{
				std::string sig = classSig.at(entityID);
				if (s_Data->EntityClasses.find(sig) == s_Data->EntityClasses.end())
				{
					instancesToRemove.push_back(entityID);
					continue;
				}

				instance = CreateRef<ScriptInstance>(s_Data->EntityClasses.at(sig), instance->m_Entity);
			}

			auto it = field_values.find(entityID);
			if (it == field_values.end())
				continue;

			auto entity_fields = it->second;
			for (const auto& [name, data] : entity_fields)
			{
				instance->SetFieldValueInternal(name, data);
				delete data;
			}
		}

		for (const UUID& id : instancesToRemove)
			instances.erase(id);
	}

	void ScriptEngine::GetScriptData(std::unordered_map<UUID, Ref<ScriptInstance>>& instances,
		fieldMap& field_values, signatureMap& classSig)
	{
		for (const auto& [entityID, instance] : instances)
		{
			const auto& monoClass = instance->GetScriptClass()->GetMonoClass();

			std::string_view name = mono_class_get_name(monoClass);
			std::string_view nameSpace = mono_class_get_namespace(monoClass);

			std::string sig = fmt::format("{}.{}", nameSpace, name);
			classSig[entityID] = sig;

			const auto& fields = instance->GetScriptClass()->GetFields();

			for (const auto& [name, field] : fields)
			{
				if (field.Type == ScriptFieldType::Entity	|| 
					field.Type == ScriptFieldType::Asset	||
					field.Type == ScriptFieldType::Prefab	||
					field.Type == ScriptFieldType::Font		||
					field.Type == ScriptFieldType::Texture	||
					field.Type == ScriptFieldType::None)
					continue;

				field_values[entityID][field.Name] = new char[16];
				instance->GetFieldValueInternal(field.Name, field_values[entityID][field.Name]);
			}
		}
	}

	/* ScriptClass || ScriptInstance */

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
		MonoObject* exception = nullptr;
		return mono_runtime_invoke(method, instance, parameters, &exception);
	}

	ScriptInstance::ScriptInstance(Ref<ScriptInstance> instance, Entity entity)
	{
		m_ScriptClass = instance->m_ScriptClass;
		m_Entity = entity;

		m_Constructor = instance->m_Constructor;
		m_OnCreateMethod = instance->m_OnCreateMethod;
		m_OnUpdateMethod = instance->m_OnUpdateMethod;
		m_OnCollisionEnterMethod = instance->m_OnCollisionEnterMethod;
		m_OnCollisionExitMethod = instance->m_OnCollisionExitMethod;

		m_Instance = mono_object_clone(instance->m_Instance);
	}

	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity) 
		: m_ScriptClass(scriptClass), m_Entity(entity)
	{
		m_Instance = m_ScriptClass->Instanciate();

		m_Constructor = s_Data->EntityClass.GetMethod(".ctor", 1);
		m_OnCreateMethod = scriptClass->GetMethod("OnCreate", 0);
		m_OnUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);
		m_OnCollisionEnterMethod = scriptClass->GetMethod("OnCollisionEnter", 1);
		m_OnCollisionExitMethod = scriptClass->GetMethod("OnCollisionExit", 1);

		UUID id = entity.GetUUID(); void* param = &id;
		m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);
	}

	void ScriptInstance::InvokeOnCreate() 
	{
		if (m_OnCreateMethod)
		{
			m_ScriptClass->InvokeMethod(m_Instance, m_OnCreateMethod);
			m_OnCreateCalled = true;
		}
	}

	void ScriptInstance::InvokeOnUpdate(float ts) 
	{
		if (m_OnUpdateMethod)
		{
			void* param = &ts;
			m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateMethod, &param);
		}
	}
	
	void ScriptInstance::InvokeOnCollisionEnter(Entity other)
	{
		if (m_OnCollisionEnterMethod)
		{
			void* param = ScriptEngine::CreateEntityClass(other.GetUUID());
			m_ScriptClass->InvokeMethod(m_Instance, m_OnCollisionEnterMethod, &param);
		}
	}
	
	void ScriptInstance::InvokeOnCollisionExit(Entity other)
	{
		if (m_OnCollisionExitMethod)
		{
			void* param = ScriptEngine::CreateEntityClass(other.GetUUID());
			m_ScriptClass->InvokeMethod(m_Instance, m_OnCollisionExitMethod, &param);
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