#pragma once

#include "Nebula/Maths/Maths.h"

#include "Nebula/Renderer/Camera.h"
#include "Scene_Camera.h"

#include "Nebula/Core/UUID.h"

#include "Nebula/Renderer/Texture.h"

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

	struct ParentChildComponent {
		UUID PrimaryParent = 0;
		
		//TODO: No Max
		UUID* ChildrenIDs = new UUID[32]; //Max 32 Children (TEMP) 
		uint32_t ChildrenCount = 0;
		
		ParentChildComponent() = default;
		ParentChildComponent(const ParentChildComponent&) = default;

		UUID operator[](int index) { return ChildrenIDs[index]; }

		void AddChild(UUID id) {
			for (uint32_t i = 0; i < ChildrenCount; i++)
				if (id == ChildrenIDs[i]) return;

			ChildrenIDs[ChildrenCount] = id;
			ChildrenCount++;
		}

		void RemoveChild(UUID id) {
			int index = -1;
			UUID* newChildren = new UUID[32];
			
			uint32_t nIndex = 0;
			for (uint32_t i = 0; i < ChildrenCount; i++) {
				if ((uint64_t)ChildrenIDs[i] != (uint64_t)id) {
					newChildren[nIndex] = ChildrenIDs[i];
					nIndex++;
				}
			}
			
			delete[] ChildrenIDs;
			ChildrenIDs = newChildren;
			ChildrenCount--;
		}
	};

	struct TransformComponent {
		vec3 LocalTranslation =	{ 0.0f, 0.0f, 0.0f };
		vec3 LocalRotation =	{ 0.0f, 0.0f, 0.0f };
		vec3 LocalScale =		{ 1.0f, 1.0f, 1.0f };

		vec3 GlobalTranslation = { 0.0f, 0.0f, 0.0f };
		vec3 GlobalRotation =	 { 0.0f, 0.0f, 0.0f };
		vec3 GlobalScale =		 { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;

		inline operator mat4() { return CalculateMatrix(); }

		inline mat4 CalculateMatrix() {
			return translate(GlobalTranslation) * toMat4(quat(GlobalRotation)) * scale(GlobalScale);
		}

		inline mat4 CalculateLocalMatrix() {
			return translate(LocalTranslation) * toMat4(quat(LocalRotation)) * scale(LocalScale);
		}

		void SetDeltaTransform(const vec3& translation, const vec3& rotation, const vec3& size) {
			LocalTranslation += translation;
			LocalRotation += rotation;
			LocalScale += size;

			GlobalTranslation += translation;
			GlobalRotation += rotation;
			GlobalScale += size;
		}
	};

	struct SpriteRendererComponent {
		vec4 Colour{ 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture = nullptr;
		float Tiling = 1.0f;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const vec4& colour, Ref<Texture2D> texture = nullptr, float tiling = 1.0f) : Colour(colour), Texture(texture), Tiling(tiling) { }
	};

	struct CircleRendererComponent {
		vec4 Colour{ 1.0f, 1.0f, 1.0f, 1.0f };
		float Radius = 0.5f;
		float Thickness = 1.0f;
		float Fade = 0.005f;

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
	};

	struct CameraComponent {
		SceneCamera Camera;
		bool Primary = true;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
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

	//Physics
	struct Rigidbody2DComponent {
		enum class BodyType { Static = 0, Dynamic, Kinematic };
		BodyType Type = BodyType::Static;
		bool FixedRotation = false;

		void* RuntimeBody = nullptr;

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
	};

	struct Box2DComponent {
		vec2 Offset = { 0.0f, 0.0f };
		vec2 Size = { 0.5f, 0.5f };

		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		void* RuntimeFixture = nullptr;

		Box2DComponent() = default;
		Box2DComponent(const Box2DComponent&) = default;
	};

	struct CircleColliderComponent {
		vec2 Offset = { 0.0f, 0.0f };
		float Radius = 0.5f;

		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		void* RuntimeFixture = nullptr;

		CircleColliderComponent() = default;
		CircleColliderComponent(const CircleColliderComponent&) = default;
	};

	template<typename... Component>
	struct ComponentGroup
	{

	};
	using AllComponents =
		ComponentGroup<ParentChildComponent, TransformComponent, SpriteRendererComponent, CircleRendererComponent,
		CameraComponent, NativeScriptComponent,
		Rigidbody2DComponent, Box2DComponent, CircleColliderComponent>; 
}