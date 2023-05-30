#pragma once

#include "Nebula/Maths/Maths.h"
#include "Nebula/Maths/Transform.h"

#include "Nebula/Renderer/Camera.h"
#include "Nebula/Renderer/Fonts.h"
#include "Scene_Camera.h"

#include "Nebula/Core/UUID.h"

#include "Nebula/AssetManager/AssetManager.h"
#include "Nebula/Project/Project.h"
#include "Nebula/Project/ProjectLayer.h"

#include "Nebula/Renderer/Texture.h"
#include "Nebula/Utils/Arrays.h"


namespace Nebula {
	struct IDComponent {
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
	};

	struct TagComponent {
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : Tag(tag) { }
	};

	struct PropertiesComponent {
		bool Enabled = true;
		Ref<ProjectLayer> Layer;
		
		PropertiesComponent() = default;
		PropertiesComponent(const PropertiesComponent&) = default;
	};

	struct ParentChildComponent {
		UUID Parent = NULL;
		Array<UUID> ChildrenIDs;
		
		ParentChildComponent() = default;
		ParentChildComponent(const ParentChildComponent&) = default;

		UUID operator[](int index) { return ChildrenIDs[index]; }

		void AddChild(UUID id) {
			ChildrenIDs.push_back(id);
		}

		void RemoveChild(UUID id) {
			ChildrenIDs.remove(id);
		}
	};

	struct TransformComponent {
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation =	{ 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;

		inline operator glm::mat4() const { return CalculateMatrix(); }
		inline glm::mat4 CalculateMatrix() const {
			return glm::translate(Translation) * glm::toMat4(glm::quat(Rotation)) * glm::scale(Scale);
		}
	};

	struct WorldTransformComponent {
		glm::mat4 Transform = glm::mat4(1.0f);

		WorldTransformComponent() = default;
		WorldTransformComponent(const WorldTransformComponent&) = default;
	};

	struct CameraComponent {
		SceneCamera Camera;
		bool Primary = true;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct ScriptComponent
	{
		std::string ClassName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;
	};

	class ScriptableEntity;
	struct NativeScriptComponent {
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity*(*InstantiateScript)();
		void (*DestroyScript)(NativeScriptComponent*);

		template<typename T>
		void Bind() {
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};

	struct SpriteRendererComponent {
		glm::vec4 Colour{ 1.0f, 1.0f, 1.0f, 1.0f };
		AssetHandle Texture = NULL;
		glm::vec2 SubTextureOffset = { 0.0f, 0.0f };
		glm::vec2 SubTextureCellSize = { 128.0f, 128.0f };
		glm::vec2 SubTextureCellNum = { 1, 1 };
		float Tiling = 1.0f;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
	};

	struct CircleRendererComponent {
		glm::vec4 Colour{ 1.0f, 1.0f, 1.0f, 1.0f };
		float Radius = 0.5f;
		float Thickness = 1.0f;
		float Fade = 0.005f;

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
	};

	struct StringRendererComponent {
		std::string Text;
		glm::vec4 Colour{ 1.0f, 1.0f, 1.0f, 1.0f };

		AssetHandle FontHandle = NULL;
		
		// For FontFamily
		bool Bold = false;
		bool Italic = false;

		float Kerning = 0.0f;
		float LineSpacing = 0.0f;

		StringRendererComponent() = default;
		StringRendererComponent(const StringRendererComponent&) = default;

		Ref<Font> GetFont() 
		{
			Ref<Asset> asset = AssetManager::GetAsset(FontHandle);
			if (!asset)
				return Font::GetDefault();

			if (asset->Type == AssetType::FontFamily)
				return asset->GetData<Font>(Bold, Italic);
			
			return asset->GetData<Font>();
		}
	};

	//Physics
	struct Rigidbody2DComponent {
		enum class BodyType { Static = 0, Dynamic, Kinematic };
		BodyType Type = BodyType::Dynamic;
		bool FixedRotation = false;
		bool Trigger = false;

		void* RuntimeBody = nullptr;

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;

		void ApplyForce(glm::vec2 force, glm::vec2 point);
		void ApplyLinearImpulse(glm::vec2 impulse, glm::vec2 point);

		void ApplyAngularImpulse(float impulse);
		void ApplyTorque(float torque);

		void ApplyForceToCenter(glm::vec2 force);
		void ApplyLinearImpulseToCenter(glm::vec2 impulse);
	};

	struct BoxCollider2DComponent {
		glm::vec2 Size = { 0.5f, 0.5f };
		glm::vec2 Offset = { 0.0f, 0.0f };

		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		uint16_t Mask = 0xFFFF;

		void* RuntimeFixture = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;

		void UpdateFilters(uint16_t Category, uint16_t Mask);
	};

	struct CircleColliderComponent {
		glm::vec2 Offset = { 0.0f, 0.0f };
		float Radius = 0.5f;

		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		uint16_t Mask = 0xFFFF;

		void* RuntimeFixture = nullptr;

		CircleColliderComponent() = default;
		CircleColliderComponent(const CircleColliderComponent&) = default;

		void UpdateFilters(uint16_t Category, uint16_t Mask);
	};

	template<typename... Component>
	struct ComponentGroup
	{

	};
	using AllComponents = ComponentGroup <
		PropertiesComponent, TransformComponent, WorldTransformComponent,
		CameraComponent, ScriptComponent, NativeScriptComponent,
		SpriteRendererComponent, CircleRendererComponent, StringRendererComponent,
		Rigidbody2DComponent, BoxCollider2DComponent, CircleColliderComponent
	>;
}