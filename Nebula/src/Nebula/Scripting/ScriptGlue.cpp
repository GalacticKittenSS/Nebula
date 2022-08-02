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

	// ENTITY CLASS

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

	// TRANSFORM COMPONENT

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

	static void TransformComponent_GetRotation(UUID entityID, vec3* out)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*out = entity.GetTransform().Rotation;
	}

	static void TransformComponent_SetRotation(UUID entityID, vec3* rotation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetTransform().Rotation = *rotation;
		UpdateChildrenAndTransform(entity);
	}

	static void TransformComponent_GetScale(UUID entityID, vec3* out)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*out = entity.GetTransform().Scale;
	}

	static void TransformComponent_SetScale(UUID entityID, vec3* scale)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetTransform().Scale = *scale;
		UpdateChildrenAndTransform(entity);
	}

	// SPRITE RENDERER COMPONENT

	static void SpriteRendererComponent_SetColour(UUID entityID, vec4* colour)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<SpriteRendererComponent>().Colour = *colour;
	}

	static void SpriteRendererComponent_GetColour(UUID entityID, vec4* colour)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*colour = entity.GetComponent<SpriteRendererComponent>().Colour;
	}

	static void SpriteRendererComponent_SetOffset(UUID entityID, vec2* offset)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<SpriteRendererComponent>().SubTextureOffset = *offset;
	}

	static void SpriteRendererComponent_GetOffset(UUID entityID, vec2* offset)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*offset = entity.GetComponent<SpriteRendererComponent>().SubTextureOffset;
	}

	static void SpriteRendererComponent_SetCellSize(UUID entityID, vec2* size)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<SpriteRendererComponent>().SubTextureCellSize = *size;
	}

	static void SpriteRendererComponent_GetCellSize(UUID entityID, vec2* size)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*size = entity.GetComponent<SpriteRendererComponent>().SubTextureCellSize;
	}

	static void SpriteRendererComponent_SetCellNumber(UUID entityID, vec2* number)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<SpriteRendererComponent>().SubTextureCellNum = *number;
	}

	static void SpriteRendererComponent_GetCellNumber(UUID entityID, vec2* number)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*number = entity.GetComponent<SpriteRendererComponent>().SubTextureCellNum;
	}

	static void SpriteRendererComponent_SetTiling(UUID entityID, float tiling)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<SpriteRendererComponent>().Tiling = tiling;
	}

	static void SpriteRendererComponent_GetTiling(UUID entityID, float* tiling)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*tiling = entity.GetComponent<SpriteRendererComponent>().Tiling;
	}

	// CIRCLE RENDERER COMPONENT

	static void CircleRendererComponent_SetColour(UUID entityID, vec4* colour)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<CircleRendererComponent>().Colour = *colour;
	}

	static void CircleRendererComponent_GetColour(UUID entityID, vec4* colour)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*colour = entity.GetComponent<CircleRendererComponent>().Colour;
	}

	static void CircleRendererComponent_SetRadius(UUID entityID, float radius)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<CircleRendererComponent>().Radius = radius;
	}

	static void CircleRendererComponent_GetRadius(UUID entityID, float* radius)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*radius = entity.GetComponent<CircleRendererComponent>().Radius;
	}

	static void CircleRendererComponent_SetThickness(UUID entityID, float thickness)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<CircleRendererComponent>().Thickness = thickness;
	}

	static void CircleRendererComponent_GetThickness(UUID entityID, float* thickness)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*thickness = entity.GetComponent<CircleRendererComponent>().Thickness;
	}

	static void CircleRendererComponent_SetFade(UUID entityID, float fade)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<CircleRendererComponent>().Fade = fade;
	}

	static void CircleRendererComponent_GetFade(UUID entityID, float* fade)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*fade = entity.GetComponent<CircleRendererComponent>().Fade;
	}

	// STRING RENDERER COMPONENT

	static void StringRendererComponent_SetText(UUID entityID, MonoString* text)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<StringRendererComponent>().Text = GetStringFromMono(text);
	}

	static void StringRendererComponent_GetText(UUID entityID, MonoString* text)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		mono_free(text);

		std::string old = entity.GetComponent<StringRendererComponent>().Text;
		text = mono_string_new(ScriptEngine::GetAppDomain(), old.c_str());
	}

	static void StringRendererComponent_SetColour(UUID entityID, vec4* colour)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<StringRendererComponent>().Colour = *colour;
	}

	static void StringRendererComponent_GetColour(UUID entityID, vec4* colour)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*colour = entity.GetComponent<StringRendererComponent>().Colour;
	}

	static void StringRendererComponent_GetResolution(UUID entityID, float resolution)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<StringRendererComponent>().Resolution = resolution;
	}

	static void StringRendererComponent_SetResolution(UUID entityID, float* resolution)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*resolution = entity.GetComponent<StringRendererComponent>().Resolution;
	}

	static void StringRendererComponent_GetBold(UUID entityID, bool bold)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<StringRendererComponent>().Bold = bold;
	}

	static void StringRendererComponent_SetBold(UUID entityID, bool* bold)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*bold = entity.GetComponent<StringRendererComponent>().Bold;
	}

	static void StringRendererComponent_GetItalic(UUID entityID, bool italic)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<StringRendererComponent>().Italic = italic;
	}

	static void StringRendererComponent_SetItalic(UUID entityID, bool* italic)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*italic = entity.GetComponent<StringRendererComponent>().Italic;
	}

	static void StringRendererComponent_GetIndex(UUID entityID, int index)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<StringRendererComponent>().FontTypeIndex = index;
	}

	static void StringRendererComponent_SetIndex(UUID entityID, int* index)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*index = entity.GetComponent<StringRendererComponent>().FontTypeIndex;
	}

	// RIGIDBODY 2D COMPONENT

	static void Rigidbody2DComponent_ApplyLinearImpulse(UUID entityID, vec2* impulse, vec2* point)
	{

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

	static void Rigidbody2DComponent_ApplyForce(UUID entityID, vec2* force, vec2* point)
	{

	}

	static void Rigidbody2DComponent_ApplyForceToCenter(UUID entityID, vec2* force)
	{

	}

	// INPUT CLASS

	static bool Input_IsKeyDown(KeyCode keycode)
	{
		return Input::IsKeyPressed(keycode);
	}

	void ScriptGlue::RegisterFunctions() {
		NB_ADD_INTERNAL_CALL(Native_Log);

		NB_ADD_INTERNAL_CALL(Entity_HasComponent);

		NB_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		NB_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		NB_ADD_INTERNAL_CALL(TransformComponent_GetScale);
		
		NB_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		NB_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		NB_ADD_INTERNAL_CALL(TransformComponent_SetScale);

		NB_ADD_INTERNAL_CALL(SpriteRendererComponent_GetCellNumber);
		NB_ADD_INTERNAL_CALL(SpriteRendererComponent_GetCellSize);
		NB_ADD_INTERNAL_CALL(SpriteRendererComponent_GetColour);
		NB_ADD_INTERNAL_CALL(SpriteRendererComponent_GetOffset);
		NB_ADD_INTERNAL_CALL(SpriteRendererComponent_GetTiling);

		NB_ADD_INTERNAL_CALL(SpriteRendererComponent_SetCellNumber);
		NB_ADD_INTERNAL_CALL(SpriteRendererComponent_SetCellSize);
		NB_ADD_INTERNAL_CALL(SpriteRendererComponent_SetColour);
		NB_ADD_INTERNAL_CALL(SpriteRendererComponent_SetOffset);
		NB_ADD_INTERNAL_CALL(SpriteRendererComponent_SetTiling);

		NB_ADD_INTERNAL_CALL(CircleRendererComponent_GetColour);
		NB_ADD_INTERNAL_CALL(CircleRendererComponent_GetFade);
		NB_ADD_INTERNAL_CALL(CircleRendererComponent_GetRadius);
		NB_ADD_INTERNAL_CALL(CircleRendererComponent_GetThickness);

		NB_ADD_INTERNAL_CALL(CircleRendererComponent_SetColour);
		NB_ADD_INTERNAL_CALL(CircleRendererComponent_SetFade);
		NB_ADD_INTERNAL_CALL(CircleRendererComponent_SetRadius);
		NB_ADD_INTERNAL_CALL(CircleRendererComponent_SetThickness);

		NB_ADD_INTERNAL_CALL(StringRendererComponent_GetBold);
		NB_ADD_INTERNAL_CALL(StringRendererComponent_GetColour);
		NB_ADD_INTERNAL_CALL(StringRendererComponent_GetIndex);
		NB_ADD_INTERNAL_CALL(StringRendererComponent_GetItalic);
		NB_ADD_INTERNAL_CALL(StringRendererComponent_GetResolution);
		NB_ADD_INTERNAL_CALL(StringRendererComponent_GetText);
		
		NB_ADD_INTERNAL_CALL(StringRendererComponent_SetBold);
		NB_ADD_INTERNAL_CALL(StringRendererComponent_SetColour);
		NB_ADD_INTERNAL_CALL(StringRendererComponent_SetIndex);
		NB_ADD_INTERNAL_CALL(StringRendererComponent_SetItalic);
		NB_ADD_INTERNAL_CALL(StringRendererComponent_SetResolution);
		NB_ADD_INTERNAL_CALL(StringRendererComponent_SetText);
		
		NB_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyForce);
		NB_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyForceToCenter);
		NB_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
		NB_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);

		NB_ADD_INTERNAL_CALL(Input_IsKeyDown);
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