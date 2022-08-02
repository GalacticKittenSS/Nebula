#include "nbpch.h"
#include "ScriptGlue.h"

#include "ScriptEngine.h"

#include "Nebula/Scene/Scene.h"
#include "Nebula/Core/Input.h"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

namespace Nebula {

#define NB_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Nebula.InternalCalls::" #Name, Name);

	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

	static std::string GetStringFromMono(MonoString* text)
	{
		char* cStr = mono_string_to_utf8(text);
		std::string str(cStr);
		mono_free(cStr);

		return str;
	}

	static void Native_Log(int level, MonoString* text)
	{
		std::string str = GetStringFromMono(text);
		
		switch (level)
		{
			case 0: NB_TRACE("[C# Script] {}", str); break;
			case 1: NB_WARN ("[C# Script] {}", str); break;
			case 2: NB_ERROR("[C# Script] {}", str); break;
		}
	}

	static bool Entity_HasComponent(UUID entityID, MonoReflectionType* componentType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		MonoType* managedTyped = mono_reflection_type_get_type(componentType);

		NB_ASSERT(s_EntityHasComponentFuncs.find(managedTyped) != s_EntityHasComponentFuncs.end());
		return s_EntityHasComponentFuncs.at(managedTyped)(entity);
	}

	static void TransformComponent_GetTranslation(UUID entityID, vec3* out)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*out = entity.GetTransform().Translation;
	}

	static void TransformComponent_SetTranslation(UUID entityID, vec3* translation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetTransform().Translation = *translation;
		UpdateChildrenAndTransform(entity);
	}

	static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID entityID, vec2* impulse)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& rigidbody = entity.GetComponent<Rigidbody2DComponent>();
		rigidbody.ApplyLinearImpulseToCenter(*impulse);
	}

	static bool Input_IsKeyDown(KeyCode keycode)
	{
		return Input::IsKeyPressed(keycode);
	}

	void ScriptGlue::RegisterFunctions() {
		NB_ADD_INTERNAL_CALL(Native_Log);
		NB_ADD_INTERNAL_CALL(Entity_HasComponent);
		NB_ADD_INTERNAL_CALL(Input_IsKeyDown);
		
		NB_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		NB_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);

		NB_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
	}

	template<typename ... Component>
	static void RegisterComponent()
	{
		([]() 
		{
			std::string_view name = typeid(Component).name();
			size_t pos = name.find_last_of(":");
			std::string_view structName = name.substr(pos + 1);
			std::string managed = fmt::format("Nebula.{}", structName);

			MonoType* managedType = mono_reflection_type_from_name(managed.data(), ScriptEngine::GetCoreAssemblyImage());
			
			if (!managedType)
			{
				NB_ERROR("Could not find component type {}", managed);
				return;
			}
			
			s_EntityHasComponentFuncs[managedType] = [](Entity entity) { return entity.HasComponent<Component>(); };
		}(), ... );
	}

	template<typename ... Component>
	static void RegisterComponent(ComponentGroup<Component ...>)
	{
		RegisterComponent<Component...>();
	}

	void ScriptGlue::RegisterComponents()
	{
		RegisterComponent(AllComponents{});
	}
}