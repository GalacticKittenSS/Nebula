#include "nbpch.h"
#include "ScriptGlue.h"

#include "ScriptEngine.h"

#include "Nebula/Core/Input.h"
#include "Nebula/Core/Application.h"
#include "Nebula/Scene/Scene.h"
#include "Nebula/Scene/Prefab_Serializer.h"
#include "Nebula/Project/Project.h"
#include "Nebula/Utils/Time.h"
#include "Nebula/Utils/Physics2D.h"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include <box2d/b2_body.h>

namespace Nebula {

#define NB_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Nebula.InternalCalls::" #Name, Name);
	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;
	static std::unordered_map<MonoType*, std::function<void(Entity)>> s_EntityAddComponentFuncs;

	namespace Utils {
		static std::string GetStringFromMono(MonoString* text)
		{
			char* cStr = mono_string_to_utf8(text);
			std::string str(cStr);
			mono_free(cStr);

			return str;
		}
	}
	
	static void Native_Log(int level, MonoString* text)
	{
		char* cStr = mono_string_to_utf8(text);
		
		switch (level)
		{
			case 0: NB_TRACE("[C# Script] {}", cStr); break;
			case 1: NB_WARN ("[C# Script] {}", cStr); break;
			case 2: NB_ERROR("[C# Script] {}", cStr); break;
		}
		
		mono_free(cStr);
	}


#pragma region Application
	static void Application_GetWindowSize(glm::vec2* size)
	{
		Window& window = Application::Get().GetWindow();
		*size = { (float)window.GetHeight(), (float)window.GetHeight() };
	}
#pragma endregion

#pragma region Mathf
	static float Mathf_ToDegrees(float radians)
	{
		return glm::degrees(radians);
	}

	static float Mathf_ToRadians(float degrees)
	{
		return glm::radians(degrees);
	}

	static float Mathf_Tan(float value)
	{
		return tan(value);
	}

	static float Mathf_Atan(float value)
	{
		return atan(value);
	}

	static float Mathf_Cos(float value)
	{
		return cos(value);
	}

	static float Mathf_Acos(float value)
	{
		return acos(value);
	}

	static float Mathf_Sin(float value)
	{
		return sin(value);
	}

	static float Mathf_Asin(float value)
	{
		return asin(value);
	}

	static float Mathf_Sqrt(float value)
	{
		return sqrt(value);
	}
#pragma endregion

#pragma region Time
	static float Time_DeltaTime()
	{
		return Time::DeltaTime();
	}
#pragma endregion

#pragma region Input
	static bool Input_IsKeyDown(KeyCode keycode)
	{
		return Input::IsKeyPressed(keycode);
	}

	static bool Input_IsMouseButtonDown(MouseCode mouseCode)
	{
		return Input::IsMouseButtonPressed(mouseCode);
	}

	static void Input_GetMousePos(glm::vec2* out)
	{
		*out = { Input::GetMouseX(), Input::GetMouseY() };
	}
#pragma endregion

#pragma region Asset
	static uint64_t Asset_GetOrCreateHandle(MonoString* path)
	{
		std::string pathString = Utils::GetStringFromMono(path);
		std::filesystem::path assetPath = Project::GetAssetFileSystemPath(pathString);

		AssetHandle handle = AssetManager::CreateAsset(assetPath);
		return handle;
	}
	
	static uint64_t Asset_GetHandleFromPath(MonoString* path)
	{
		std::string pathString = Utils::GetStringFromMono(path);
		std::filesystem::path assetPath = Project::GetAssetFileSystemPath(pathString);

		AssetHandle handle = AssetManager::GetHandleFromPath(assetPath);
		return handle;
	}

	static MonoString* Asset_GetPathFromHandle(AssetHandle handle)
	{
		const AssetMetadata& metadata = AssetManager::GetAssetMetadata(handle);
		if (!metadata)
			return nullptr;

		std::string path = metadata.RelativePath.string();
		return ScriptEngine::CreateMonoString(path.c_str());
	}
#pragma endregion

#pragma region Font
	static bool Font_GetBold(AssetHandle handle)
	{
		if (!handle)
			return NULL;

		Ref<Font> font = AssetManager::GetAsset<Font>(handle);
		NB_ASSERT(font);

		// font->IsBold()
		return false;
	}

	static bool Font_GetItalic(AssetHandle handle)
	{
		if (!handle)
			return NULL;

		Ref<Font> font = AssetManager::GetAsset<Font>(handle);
		NB_ASSERT(font);

		// font->IsItalic()
		return false;
	}
#pragma endregion

#pragma region Scene
	static uint64_t Scene_FindEntityByName(MonoString* name)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);

		char* cName = mono_string_to_utf8(name);
		Entity entity = scene->GetEntityWithTag(cName);
		mono_free(cName);

		if (!entity)
			return 0;

		return entity.GetUUID();
	}

	static uint64_t Scene_CreateNewEntity(MonoString* name)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);

		char* cName = mono_string_to_utf8(name);
		Entity entity = scene->CreateEntity(cName);
		mono_free(cName);

		if (!entity)
			return 0;

		return entity.GetUUID();
	}

	static uint64_t Scene_DuplicateEntity(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		Entity duplicate = scene->DuplicateEntity(entity);
		return duplicate.GetUUID();
	}

	static void Scene_DestroyEntity(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };

		if (entity)
		{
			scene->DestroyEntity(entity);
		}
	}
#pragma endregion

#pragma region Entity
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

	static void Entity_AddComponent(UUID entityID, MonoReflectionType* componentType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		MonoType* managedTyped = mono_reflection_type_get_type(componentType);

		NB_ASSERT(s_EntityAddComponentFuncs.find(managedTyped) != s_EntityAddComponentFuncs.end());
		s_EntityAddComponentFuncs.at(managedTyped)(entity);
	}

	static MonoString* Entity_GetName(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		std::string name = entity.GetName();
		return ScriptEngine::CreateMonoString(name.c_str());
	}

	static void Entity_SetName(UUID entityID, MonoString* name)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<TagComponent>();
		component.Tag = Utils::GetStringFromMono(name);
	}

	static MonoObject* Entity_GetScriptInstance(UUID entityID)
	{
		return ScriptEngine::GetManagedInstance(entityID);
	}

	static MonoObject* Entity_SetScriptInstance(UUID entityID, MonoReflectionType* scriptType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		MonoClass* monoClass = mono_type_get_class(mono_reflection_type_get_type(scriptType));
		std::string name = mono_class_get_name(monoClass);
		std::string nameSpace = mono_class_get_namespace(monoClass);

		auto& component = entity.AddOrReplaceComponent<ScriptComponent>();
		component.ClassName = fmt::format("{}.{}", nameSpace, name);

		Ref<ScriptInstance> instance = ScriptEngine::CreateScriptInstance(entity);
		ScriptEngine::OnCreateEntity(entity);

		return instance->GetManagedObject();
	}

	static uint64_t Entity_FindChildByName(UUID entityID, MonoString* name)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		UUID childID = 0;
		char* cName = mono_string_to_utf8(name);

		const auto& children = entity.GetParentChild().ChildrenIDs;
		for (const auto& id : children) {
			Entity ent = { id, scene };

			if (ent.GetName() == cName)
			{
				childID = ent.GetUUID();
				break;
			}
		}

		if (childID == 0)
			NB_ERROR("Could not find child with name {0}", cName);

		mono_free(cName);
		return childID;
	}

	static uint16_t Entity_GetLayer(uint64_t entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& comp = entity.GetComponent<PropertiesComponent>();
		return comp.Layer->Identity;
	}

	static void Entity_SetLayer(uint64_t entityID, uint16_t layer)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& comp = entity.GetComponent<PropertiesComponent>();
		comp.Layer->Identity = layer;
	}

	static uint64_t Entity_GetChild(UUID entityID, uint32_t index)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& comp = entity.GetComponent<ParentChildComponent>();
		if (index > comp.ChildrenIDs.size())
			return NULL;

		return comp.ChildrenIDs[index];
	}

	static uint32_t Entity_GetChildCount(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& comp = entity.GetComponent<ParentChildComponent>();
		return (uint32_t)comp.ChildrenIDs.size();
	}
#pragma endregion

#pragma region Prefab
	static uint64_t Prefab_Create(AssetHandle handle)
	{
		if (!handle)
			return NULL;

		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		const AssetMetadata& metadata = AssetManager::GetAssetMetadata(handle);
		NB_ASSERT(metadata);

		PrefabSerializer serializer(scene);
		Entity entity = serializer.Deserialize(metadata.Path.string());
		
		if (!entity)
			return NULL;

		return entity.GetUUID();
	}
#pragma endregion

#pragma region TransformComponent
	static void TransformComponent_GetTranslation(UUID entityID, glm::vec3* out)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*out = entity.GetTransform().Translation;
	}

	static void TransformComponent_SetTranslation(UUID entityID, glm::vec3* translation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetTransform().Translation = *translation;
		entity.UpdateTransform();
	}

	static void TransformComponent_GetRotation(UUID entityID, glm::vec3* out)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*out = entity.GetTransform().Rotation;
	}

	static void TransformComponent_SetRotation(UUID entityID, glm::vec3* rotation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetTransform().Rotation = *rotation;
		entity.UpdateTransform();
	}

	static void TransformComponent_GetScale(UUID entityID, glm::vec3* out)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*out = entity.GetTransform().Scale;
	}

	static void TransformComponent_SetScale(UUID entityID, glm::vec3* scale)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetTransform().Scale = *scale;
		entity.UpdateTransform();
	}

	static void TransformComponent_GetWorldTranslation(UUID entityID, glm::vec3* out)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		WorldTransformComponent& world = entity.GetComponent<WorldTransformComponent>();
		*out = world.Transform[3];
	}

	static void TransformComponent_GetWorldRotation(UUID entityID, glm::vec3* out)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		glm::vec3 translation, scale;
		WorldTransformComponent& world = entity.GetComponent<WorldTransformComponent>();
		Maths::DecomposeTransform(world.Transform, translation, *out, scale);
	}

	static void TransformComponent_GetWorldScale(UUID entityID, glm::vec3* out)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		glm::vec3 translation, rotation;
		WorldTransformComponent& world = entity.GetComponent<WorldTransformComponent>();
		Maths::DecomposeTransform(world.Transform, translation, rotation, *out);
	}
#pragma endregion

#pragma region CameraComponent
	static bool CameraComponent_GetPrimary(UUID entityID) 
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<CameraComponent>();
		return component.Primary;
	}

	static void CameraComponent_SetPrimary(UUID entityID, bool primary)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<CameraComponent>();
		component.Primary = primary;
	}

	static bool CameraComponent_GetFixedRatio(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<CameraComponent>();
		return component.FixedAspectRatio;
	}

	static void CameraComponent_SetFixedRatio(UUID entityID, bool fixedRatio)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<CameraComponent>();
		component.FixedAspectRatio = fixedRatio;
	}

	static void CameraComponent_ScreenToWorld(UUID entityID, glm::vec2 input, glm::vec3* out)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& camComp = entity.GetComponent<CameraComponent>();
		auto& transform = entity.GetComponent<WorldTransformComponent>();

		float z = transform.Transform[3].z;

		glm::mat4 iProj = glm::inverse(camComp.Camera.GetProjection());
		*out = iProj * glm::vec4(input, 0.0f, 1.0f) * z;
		*out = transform.Transform * glm::vec4(*out, 1.0f);
	}

	static void CameraComponent_WorldToScreen(UUID entityID, glm::vec3 point, glm::vec2* out)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& camComp = entity.GetComponent<CameraComponent>();
		auto& transform = entity.GetComponent<WorldTransformComponent>();

		glm::mat4 iProj = camComp.Camera.GetProjection() * glm::inverse(transform.Transform);
		*out = iProj * glm::vec4(point, 1.0f);
	}
#pragma endregion

#pragma region ScriptComponent
	static MonoString* ScriptComponent_GetClass(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		std::string name = entity.GetComponent<ScriptComponent>().ClassName;
		return ScriptEngine::CreateMonoString(name.c_str());
	}

	static void ScriptComponent_SetClass(UUID entityID, MonoString* name)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<ScriptComponent>();
		component.ClassName = Utils::GetStringFromMono(name);
	}
#pragma endregion

#pragma region SpriteRendererComponent
	static void SpriteRendererComponent_SetColour(UUID entityID, glm::vec4* colour)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<SpriteRendererComponent>().Colour = *colour;
	}

	static void SpriteRendererComponent_GetColour(UUID entityID, glm::vec4* colour)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*colour = entity.GetComponent<SpriteRendererComponent>().Colour;
	}

	static void SpriteRendererComponent_SetOffset(UUID entityID, glm::vec2* offset)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<SpriteRendererComponent>().SubTextureOffset = *offset;
	}

	static void SpriteRendererComponent_GetOffset(UUID entityID, glm::vec2* offset)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*offset = entity.GetComponent<SpriteRendererComponent>().SubTextureOffset;
	}

	static void SpriteRendererComponent_SetCellSize(UUID entityID, glm::vec2* size)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<SpriteRendererComponent>().SubTextureCellSize = *size;
	}

	static void SpriteRendererComponent_GetCellSize(UUID entityID, glm::vec2* size)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*size = entity.GetComponent<SpriteRendererComponent>().SubTextureCellSize;
	}

	static void SpriteRendererComponent_SetCellNumber(UUID entityID, glm::vec2* number)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<SpriteRendererComponent>().SubTextureCellNum = *number;
	}

	static void SpriteRendererComponent_GetCellNumber(UUID entityID, glm::vec2* number)
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

	static float SpriteRendererComponent_GetTiling(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		return entity.GetComponent<SpriteRendererComponent>().Tiling;
	}
#pragma endregion

#pragma region CircleRendererComponent
	static void CircleRendererComponent_SetColour(UUID entityID, glm::vec4* colour)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<CircleRendererComponent>().Colour = *colour;
	}

	static void CircleRendererComponent_GetColour(UUID entityID, glm::vec4* colour)
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

	static float CircleRendererComponent_GetRadius(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		return entity.GetComponent<CircleRendererComponent>().Radius;
	}

	static void CircleRendererComponent_SetThickness(UUID entityID, float thickness)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<CircleRendererComponent>().Thickness = thickness;
	}

	static float CircleRendererComponent_GetThickness(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		return entity.GetComponent<CircleRendererComponent>().Thickness;
	}

	static void CircleRendererComponent_SetFade(UUID entityID, float fade)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		entity.GetComponent<CircleRendererComponent>().Fade = fade;
	}

	static float CircleRendererComponent_GetFade(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		return entity.GetComponent<CircleRendererComponent>().Fade;
	}
#pragma endregion

#pragma region StringRendererComponent
	static void StringRendererComponent_SetText(UUID entityID, MonoString* text)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<StringRendererComponent>();
		component.Text = Utils::GetStringFromMono(text);
	}

	static MonoString* StringRendererComponent_GetText(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		std::string text = entity.GetComponent<StringRendererComponent>().Text;
		return ScriptEngine::CreateMonoString(text.c_str());
	}

	static void StringRendererComponent_SetColour(UUID entityID, glm::vec4* colour)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<StringRendererComponent>();
		component.Colour = *colour;
	}

	static void StringRendererComponent_GetColour(UUID entityID, glm::vec4* colour)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		*colour = entity.GetComponent<StringRendererComponent>().Colour;
	}

	static void StringRendererComponent_SetFontHandle(UUID entityID, AssetHandle handle)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<StringRendererComponent>();
		component.FontHandle = handle;
	}

	static uint64_t StringRendererComponent_GetFontHandle(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<StringRendererComponent>();
		return component.FontHandle;
	}

	static void StringRendererComponent_SetBold(UUID entityID, bool bold)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<StringRendererComponent>();
		component.Bold = bold;
	}

	static bool StringRendererComponent_GetBold(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		return entity.GetComponent<StringRendererComponent>().Bold;
	}

	static void StringRendererComponent_SetItalic(UUID entityID, bool italic)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<StringRendererComponent>();
		component.Italic = italic;
	}

	static bool StringRendererComponent_GetItalic(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		return entity.GetComponent<StringRendererComponent>().Italic;
	}

	static void StringRendererComponent_SetKerning(UUID entityID, float kerning)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<StringRendererComponent>();
		component.Kerning = kerning;
	}

	static float StringRendererComponent_GetKerning(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<StringRendererComponent>();
		return component.Kerning;
	}
	
	static void StringRendererComponent_SetLineSpacing(UUID entityID, float lineSpacing)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<StringRendererComponent>();
		component.LineSpacing = lineSpacing;
	}
	
	static float StringRendererComponent_GetLineSpacing(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<StringRendererComponent>();
		return component.LineSpacing;
	}
#pragma endregion

#pragma region Rigidbody2DComponent
	static int Rigidbody2DComponent_GetBodyType(UUID entityID) {
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<Rigidbody2DComponent>();
		return (int)component.Type;
	}

	static void Rigidbody2DComponent_SetBodyType(UUID entityID, int type) 
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<Rigidbody2DComponent>();
		component.Type = (Rigidbody2DComponent::BodyType)type;

		b2Body* body = (b2Body*)component.RuntimeBody;
		body->SetType(Utils::Rigibody2DToBox2D(component.Type));
	}

	static bool Rigidbody2DComponent_GetFixedRotation(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<Rigidbody2DComponent>();
		return component.FixedRotation;
	}

	static void Rigidbody2DComponent_SetFixedRotation(UUID entityID, bool fixedRotation) 
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<Rigidbody2DComponent>();
		component.FixedRotation = fixedRotation;
	}

	static void Rigidbody2DComponent_ApplyLinearImpulse(UUID entityID, glm::vec2* impulse, glm::vec2* point)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& rigidbody = entity.GetComponent<Rigidbody2DComponent>();
		rigidbody.ApplyLinearImpulse(*impulse, *point);
	}

	static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID entityID, glm::vec2* impulse)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& rigidbody = entity.GetComponent<Rigidbody2DComponent>();
		rigidbody.ApplyLinearImpulseToCenter(*impulse);
	}

	static void Rigidbody2DComponent_GetLinearVelocity(UUID entityID, glm::vec2* out)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& rigidbody = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rigidbody.RuntimeBody;
		const b2Vec2& linearVecolity = body->GetLinearVelocity();
		*out = glm::vec2(linearVecolity.x, linearVecolity.y);
	}

	static void Rigidbody2DComponent_ApplyForce(UUID entityID, glm::vec2* force, glm::vec2* point)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& rigidbody = entity.GetComponent<Rigidbody2DComponent>();
		rigidbody.ApplyForce(*force, *point);
	}

	static void Rigidbody2DComponent_ApplyForceToCenter(UUID entityID, glm::vec2* force)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& rigidbody = entity.GetComponent<Rigidbody2DComponent>();
		rigidbody.ApplyForceToCenter(*force);
	}
#pragma endregion

#pragma region BoxCollider2DComponent
	static void BoxCollider2DComponent_GetSize(UUID entityID, glm::vec2* size)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<BoxCollider2DComponent>();
		*size = component.Size;
	}

	static void BoxCollider2DComponent_SetSize(UUID entityID, glm::vec2* size)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<BoxCollider2DComponent>();
		component.Size = *size;
	}

	static void BoxCollider2DComponent_GetOffset(UUID entityID, glm::vec2* offset)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<BoxCollider2DComponent>();
		*offset = component.Offset;
	}

	static void BoxCollider2DComponent_SetOffset(UUID entityID, glm::vec2* offset)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<BoxCollider2DComponent>();
		component.Offset = *offset;
	}

	static float BoxCollider2DComponent_GetDensity(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<BoxCollider2DComponent>();
		return component.Density;
	}

	static void BoxCollider2DComponent_SetDensity(UUID entityID, float density)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<BoxCollider2DComponent>();
		component.Density = density;
	}

	static float BoxCollider2DComponent_GetFriction(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<BoxCollider2DComponent>();
		return component.Friction;
	}

	static void BoxCollider2DComponent_SetFriction(UUID entityID, float friction)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<BoxCollider2DComponent>();
		component.Friction = friction;
	}

	static float BoxCollider2DComponent_GetRestitution(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<BoxCollider2DComponent>();
		return component.Restitution;
	}

	static void BoxCollider2DComponent_SetRestitution(UUID entityID, float restitution)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<BoxCollider2DComponent>();
		component.Restitution = restitution;
	}

	static float BoxCollider2DComponent_GetThreshold(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<BoxCollider2DComponent>();
		return component.RestitutionThreshold;
	}

	static void BoxCollider2DComponent_SetThreshold(UUID entityID, float threshold)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<BoxCollider2DComponent>();
		component.RestitutionThreshold = threshold;
	}
	
	static uint16_t BoxCollider2DComponent_GetMask(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<BoxCollider2DComponent>();
		return component.Mask;
	}

	static void BoxCollider2DComponent_SetMask(UUID entityID, uint16_t mask)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<BoxCollider2DComponent>();
		component.Mask = mask;
	}
#pragma endregion

#pragma region CircleCollider2DComponent
	static float CircleCollider2DComponent_GetRadius(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<CircleColliderComponent>();
		return component.Radius;
	}

	static void CircleCollider2DComponent_SetRadius(UUID entityID, float radius)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<CircleColliderComponent>();
		component.Radius = radius;
	}

	static void CircleCollider2DComponent_GetOffset(UUID entityID, glm::vec2* offset)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<CircleColliderComponent>();
		*offset = component.Offset;
	}

	static void CircleCollider2DComponent_SetOffset(UUID entityID, glm::vec2* offset)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<CircleColliderComponent>();
		component.Offset = *offset;
	}

	static float CircleCollider2DComponent_GetDensity(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<CircleColliderComponent>();
		return component.Density;
	}

	static void CircleCollider2DComponent_SetDensity(UUID entityID, float density)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<CircleColliderComponent>();
		component.Density = density;
	}

	static float CircleCollider2DComponent_GetFriction(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<CircleColliderComponent>();
		return component.Friction;
	}

	static void CircleCollider2DComponent_SetFriction(UUID entityID, float friction)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<CircleColliderComponent>();
		component.Friction = friction;
	}

	static float CircleCollider2DComponent_GetRestitution(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<CircleColliderComponent>();
		return component.Restitution;
	}

	static void CircleCollider2DComponent_SetRestitution(UUID entityID, float restitution)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<CircleColliderComponent>();
		component.Restitution = restitution;
	}

	static float CircleCollider2DComponent_GetThreshold(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<CircleColliderComponent>();
		return component.RestitutionThreshold;
	}

	static void CircleCollider2DComponent_SetThreshold(UUID entityID, float threshold)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<CircleColliderComponent>();
		component.RestitutionThreshold = threshold;
	}

	static uint16_t CircleCollider2DComponent_GetMask(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<BoxCollider2DComponent>();
		return component.Mask;
	}

	static void CircleCollider2DComponent_SetMask(UUID entityID, uint16_t mask)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NB_ASSERT(scene);
		Entity entity = { entityID, scene };
		NB_ASSERT(entity);

		auto& component = entity.GetComponent<BoxCollider2DComponent>();
		component.Mask = mask;
	}
#pragma endregion

	void ScriptGlue::RegisterFunctions() {
		NB_ADD_INTERNAL_CALL(Native_Log);

		NB_ADD_INTERNAL_CALL(Application_GetWindowSize);

		NB_ADD_INTERNAL_CALL(Mathf_ToDegrees);
		NB_ADD_INTERNAL_CALL(Mathf_ToRadians);
		NB_ADD_INTERNAL_CALL(Mathf_Tan);
		NB_ADD_INTERNAL_CALL(Mathf_Atan);
		NB_ADD_INTERNAL_CALL(Mathf_Cos);
		NB_ADD_INTERNAL_CALL(Mathf_Acos);
		NB_ADD_INTERNAL_CALL(Mathf_Sin);
		NB_ADD_INTERNAL_CALL(Mathf_Asin);
		NB_ADD_INTERNAL_CALL(Mathf_Sqrt);

		NB_ADD_INTERNAL_CALL(Time_DeltaTime);

		NB_ADD_INTERNAL_CALL(Input_IsKeyDown);
		NB_ADD_INTERNAL_CALL(Input_IsMouseButtonDown);
		NB_ADD_INTERNAL_CALL(Input_GetMousePos);

		NB_ADD_INTERNAL_CALL(CameraComponent_ScreenToWorld);
		NB_ADD_INTERNAL_CALL(CameraComponent_WorldToScreen);
		
		NB_ADD_INTERNAL_CALL(Asset_GetHandleFromPath);
		NB_ADD_INTERNAL_CALL(Asset_GetPathFromHandle);
		NB_ADD_INTERNAL_CALL(Asset_GetOrCreateHandle);

		NB_ADD_INTERNAL_CALL(Font_GetBold);
		NB_ADD_INTERNAL_CALL(Font_GetItalic);

		NB_ADD_INTERNAL_CALL(Scene_FindEntityByName);
		NB_ADD_INTERNAL_CALL(Scene_CreateNewEntity);
		NB_ADD_INTERNAL_CALL(Scene_DuplicateEntity);
		NB_ADD_INTERNAL_CALL(Scene_DestroyEntity);

		NB_ADD_INTERNAL_CALL(Prefab_Create);

		NB_ADD_INTERNAL_CALL(Entity_HasComponent);
		NB_ADD_INTERNAL_CALL(Entity_AddComponent);
		NB_ADD_INTERNAL_CALL(Entity_GetName);
		NB_ADD_INTERNAL_CALL(Entity_SetName);
		NB_ADD_INTERNAL_CALL(Entity_GetScriptInstance);
		NB_ADD_INTERNAL_CALL(Entity_SetScriptInstance);
		NB_ADD_INTERNAL_CALL(Entity_FindChildByName);
		NB_ADD_INTERNAL_CALL(Entity_GetChild);
		NB_ADD_INTERNAL_CALL(Entity_GetChildCount);

		NB_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		NB_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		NB_ADD_INTERNAL_CALL(TransformComponent_GetScale);

		NB_ADD_INTERNAL_CALL(TransformComponent_GetWorldTranslation);
		NB_ADD_INTERNAL_CALL(TransformComponent_GetWorldRotation);
		NB_ADD_INTERNAL_CALL(TransformComponent_GetWorldScale);
		NB_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		NB_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		NB_ADD_INTERNAL_CALL(TransformComponent_SetScale);

		NB_ADD_INTERNAL_CALL(CameraComponent_GetFixedRatio);
		NB_ADD_INTERNAL_CALL(CameraComponent_GetPrimary);
		NB_ADD_INTERNAL_CALL(CameraComponent_SetFixedRatio);
		NB_ADD_INTERNAL_CALL(CameraComponent_SetPrimary);

		NB_ADD_INTERNAL_CALL(ScriptComponent_GetClass);
		NB_ADD_INTERNAL_CALL(ScriptComponent_SetClass);

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
		NB_ADD_INTERNAL_CALL(StringRendererComponent_GetFontHandle);
		NB_ADD_INTERNAL_CALL(StringRendererComponent_GetItalic);
		NB_ADD_INTERNAL_CALL(StringRendererComponent_GetText);
		NB_ADD_INTERNAL_CALL(StringRendererComponent_GetKerning);
		NB_ADD_INTERNAL_CALL(StringRendererComponent_GetLineSpacing);
		
		NB_ADD_INTERNAL_CALL(StringRendererComponent_SetBold);
		NB_ADD_INTERNAL_CALL(StringRendererComponent_SetColour);
		NB_ADD_INTERNAL_CALL(StringRendererComponent_SetFontHandle);
		NB_ADD_INTERNAL_CALL(StringRendererComponent_SetItalic);
		NB_ADD_INTERNAL_CALL(StringRendererComponent_SetText);
		NB_ADD_INTERNAL_CALL(StringRendererComponent_SetKerning);
		NB_ADD_INTERNAL_CALL(StringRendererComponent_SetLineSpacing);
		
		NB_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetBodyType);
		NB_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetFixedRotation);
		NB_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetBodyType);
		NB_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetFixedRotation);

		NB_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyForce);
		NB_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyForceToCenter);
		NB_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
		NB_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
		NB_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetLinearVelocity);

		NB_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetDensity);
		NB_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetFriction);
		NB_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetOffset);
		NB_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetRestitution);
		NB_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetSize);
		NB_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetThreshold);
		NB_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetMask);

		NB_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetDensity);
		NB_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetFriction);
		NB_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetOffset);
		NB_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetRestitution);
		NB_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetSize);
		NB_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetThreshold);
		NB_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetMask);

		NB_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetDensity);
		NB_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetFriction);
		NB_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetOffset);
		NB_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetRestitution);
		NB_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetRadius);
		NB_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetThreshold);
		NB_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetMask);

		NB_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetDensity);
		NB_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetFriction);
		NB_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetOffset);
		NB_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetRestitution);
		NB_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetRadius);
		NB_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetThreshold);
		NB_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetMask);
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
			s_EntityAddComponentFuncs[managedType] = [](Entity entity) { entity.AddComponent<Component>(); };
		}(), ... );
	}

	template<typename ... Component>
	static void RegisterComponent(ComponentGroup<Component ...>)
	{
		RegisterComponent<Component...>();
	}

	void ScriptGlue::RegisterComponents()
	{
		s_EntityAddComponentFuncs.clear();
		s_EntityHasComponentFuncs.clear();

		RegisterComponent(AllComponents{});
	}
}