#pragma once

#include <map>

#include "Nebula/Scene/Entity.h"
#include "Nebula/Scene/Scene.h"

extern "C" {
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoDomain MonoDomain;
	typedef struct _MonoClassField MonoClassField;
}

namespace Nebula {
	enum class ScriptFieldType
	{
		None = 0,
		
		Float, Double,
		Bool, Char, 
		Byte, Short, Int, Long,
		SByte, UShort, UInt, ULong,

		Vector2, Vector3, Vector4,
		Entity
	};

	struct ScriptField
	{
		ScriptFieldType Type;
		std::string Name;
		MonoClassField* ClassField;
	};

	// Script Field + data storage
	class ScriptFieldInstance
	{
	public:
		ScriptField Field;
		
		ScriptFieldInstance()
		{
			memset(m_Buffer, 0, sizeof(m_Buffer));
		}

		template<typename T>
		T GetValue() const
		{
			static_assert(sizeof(T) <= 16, "Type too large");
			return *(T*)m_Buffer;
		}

		template<typename T>
		void SetValue(const T& value)
		{
			static_assert(sizeof(T) <= 16, "Type too large");
			memcpy(m_Buffer, &value, sizeof(T));
		}
	private:
		uint8_t m_Buffer[16];
		friend class ScriptEngine;
	};

	using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;

	class ScriptClass {
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore = false);

		MonoClass* GetMonoClass() { return m_MonoClass; }

		MonoObject* Instanciate();
		MonoMethod* GetMethod(const std::string& name, int parameterCount);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** parameters = nullptr);

		const std::map<std::string, ScriptField> GetFields() const { return m_Fields; }
	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;

		std::map<std::string, ScriptField> m_Fields;

		MonoClass* m_MonoClass = nullptr;

		friend class ScriptEngine;
	};

	class ScriptInstance
	{
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);

		void InvokeOnCreate();
		void InvokeOnUpdate(float ts);

		Ref<ScriptClass> GetScriptClass() { return m_ScriptClass; }
		MonoObject* GetManagedObject() { return m_Instance; }
	
		template<typename T>
		T GetFieldValue(const std::string& name)
		{
			static_assert(sizeof(T) <= 16, "Type too large");
			
			bool success = GetFieldValueInternal(name, s_FieldValueBuffer);
			if (!success)
				return T();

			return *(T*)s_FieldValueBuffer;
		}
	
		template<typename T>
		void SetFieldValue(const std::string& name, const T& value)
		{
			static_assert(sizeof(T) <= 16, "Type too large");
			SetFieldValueInternal(name, &value);
		}
	private:
		bool GetFieldValueInternal(const std::string& name, void* buffer);
		bool SetFieldValueInternal(const std::string& name, const void* value);
	private:
		Ref<ScriptClass> m_ScriptClass;

		MonoObject* m_Instance = nullptr;
		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;

		inline static char s_FieldValueBuffer[16];

		friend class ScriptEngine;
	};

	class ScriptEngine {
	public:
		static void Init();
		static void Shutdown();

		static void LoadAssembly(const std::filesystem::path& filepath);
		static void LoadAppAssembly(const std::filesystem::path& filepath);

		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();

		static void OnCreateEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, float ts);

		static bool EntityClassExists(const std::string& signature);
		static Scene* GetSceneContext();

		static const std::unordered_map<std::string, Ref<ScriptClass>> GetEntityClasses();
		static Ref<ScriptClass> GetEntityClass(std::string name);
		
		static ScriptFieldMap& GetScriptFieldMap(UUID id);
		static Ref<ScriptInstance> GetEntityScriptInstance(UUID id);

		static MonoImage* GetCoreAssemblyImage();
		static MonoDomain* GetAppDomain();

		static MonoObject* GetManagedInstance(UUID uuid);
	private:
		static void InitMono();
		static void ShutdownMono();

		static void LoadAssemblyClasses();

		static MonoObject* InstanciateClass(MonoClass* monoClass);

		friend class ScriptClass;
		friend class ScriptGlue;
	};

	namespace Utils {
		inline const char* ScriptFieldTypeToString(ScriptFieldType fieldType)
		{
			switch (fieldType)
			{
			case Nebula::ScriptFieldType::None:		return "None";
			case Nebula::ScriptFieldType::Float:	return "float";
			case Nebula::ScriptFieldType::Double:	return "double";
			case Nebula::ScriptFieldType::Bool:		return "bool";
			case Nebula::ScriptFieldType::Char:		return "char";
			case Nebula::ScriptFieldType::Byte:		return "byte";
			case Nebula::ScriptFieldType::Short:	return "short";
			case Nebula::ScriptFieldType::Int:		return "int";
			case Nebula::ScriptFieldType::Long:		return "long";
			case Nebula::ScriptFieldType::SByte:	return "sbyte";
			case Nebula::ScriptFieldType::UShort:	return "ushort";
			case Nebula::ScriptFieldType::UInt:		return "uint";
			case Nebula::ScriptFieldType::ULong:	return "ulong";
			case Nebula::ScriptFieldType::Vector2:	return "Vector2";
			case Nebula::ScriptFieldType::Vector3:	return "Vector3";
			case Nebula::ScriptFieldType::Vector4:	return "Vector4";
			case Nebula::ScriptFieldType::Entity:	return "Entity";
			}

			NB_ASSERT(false);
			return "None";
		}
		
		inline ScriptFieldType ScriptFieldTypeFromString(std::string_view fieldType)
		{
			if (fieldType == "none")	return ScriptFieldType::None;
			if (fieldType == "float")	return ScriptFieldType::Float;
			if (fieldType == "double")	return ScriptFieldType::Double;
			if (fieldType == "bool")	return ScriptFieldType::Bool;
			if (fieldType == "char")	return ScriptFieldType::Char;
			if (fieldType == "byte")	return ScriptFieldType::Byte;
			if (fieldType == "short")	return ScriptFieldType::Short;
			if (fieldType == "int")		return ScriptFieldType::Int;
			if (fieldType == "long")	return ScriptFieldType::Long;
			if (fieldType == "sbyte")	return ScriptFieldType::SByte;
			if (fieldType == "ushort")	return ScriptFieldType::UShort;
			if (fieldType == "uint")	return ScriptFieldType::UInt;
			if (fieldType == "ulong")	return ScriptFieldType::ULong;
			if (fieldType == "Vector2") return ScriptFieldType::Vector2;
			if (fieldType == "Vector3") return ScriptFieldType::Vector3;
			if (fieldType == "Vector4") return ScriptFieldType::Vector4;
			if (fieldType == "Entity")	return ScriptFieldType::Entity;

			NB_ASSERT(false);
			return ScriptFieldType::None;
		}
	}
}