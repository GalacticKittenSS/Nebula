#pragma once

#include "Nebula/Maths/Maths.h"

#include "Nebula/Renderer/Camera.h"
#include "Nebula/Renderer/Fonts.h"
#include "Scene_Camera.h"

#include "Nebula/Core/UUID.h"

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
		vec3 LocalTranslation =	{ 0.0f, 0.0f, 0.0f };
		vec3 LocalRotation =	{ 0.0f, 0.0f, 0.0f };
		vec3 LocalScale =		{ 1.0f, 1.0f, 1.0f };

		vec3 GlobalTranslation = { 0.0f, 0.0f, 0.0f };
		vec3 GlobalRotation =	 { 0.0f, 0.0f, 0.0f };
		vec3 GlobalScale =		 { 1.0f, 1.0f, 1.0f };

		mat4 global = mat4(1.0f);
		mat4 local = mat4(1.0f);

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;

		inline operator mat4() { return CalculateMatrix(); }

		void UpdateMatrix() {
			CalculateMatrix();
			CalculateLocalMatrix();
		}

		inline mat4 CalculateMatrix() {
			global = translate(GlobalTranslation) * toMat4(quat(GlobalRotation)) * scale(GlobalScale);
			return global;
		}

		inline mat4 CalculateLocalMatrix() {
			local = translate(LocalTranslation) * toMat4(quat(LocalRotation)) * scale(LocalScale);
			return local;
		}

		void SetDeltaTransform(const vec3& translation, const vec3& rotation, const vec3& size) {
			LocalTranslation += translation;
			LocalRotation += rotation;
			LocalScale += size;

			GlobalTranslation += translation;
			GlobalRotation += rotation;
			GlobalScale += size;

			UpdateMatrix();
		}
	};

	struct SpriteRendererComponent {
		vec4 Colour{ 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture = nullptr;
		vec2 SubTextureOffset = { 0.0f, 0.0f };
		vec2 SubTextureCellSize = { 128.0f, 128.0f };
		vec2 SubTextureCellNum = { 1, 1 };
		float Tiling = 1.0f;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
	};

	struct CircleRendererComponent {
		vec4 Colour{ 1.0f, 1.0f, 1.0f, 1.0f };
		float Radius = 0.5f;
		float Thickness = 1.0f;
		float Fade = 0.005f;

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
	};

#define StringRenderFontTypeStrings { "Default", "OpenSans", "Roboto" }
	struct StringRendererComponent {
		std::string Text;
		vec4 Colour{ 1.0f, 1.0f, 1.0f, 1.0f };

		float Resolution = 86.0f;
		bool Bold = false;
		bool Italic = false;

		int FontTypeIndex = 0;
		Font* Ft = nullptr;

		StringRendererComponent() = default;
		StringRendererComponent(const StringRendererComponent&) = default;

		void InitiateFont() {
			static const char* fontStrings[] = StringRenderFontTypeStrings;
			std::string name = fontStrings[FontTypeIndex];
			if (name == "Default")
				name = "OpenSans";

			std::string filepath = "Resources/fonts/" + name + "/";

			if (Bold) {
				filepath += "Bold";
				name += " Bold";
			}

			if (Italic) {
				filepath += "Italic";
				name += " Italic";
			}

			if (!Bold && !Italic)
				filepath += "Regular";

			Ft = new Font(name, filepath + ".ttf", Resolution);
		}
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
		BodyType Type = BodyType::Dynamic;
		bool FixedRotation = false;

		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		void* RuntimeBody = nullptr;

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;

		void ApplyForce(vec2 force, vec2 point);
		void ApplyLinearImpulse(vec2 impulse, vec2 point);

		void ApplyAngularImpulse(float impulse);
		void ApplyTorque(float torque);

		void ApplyForceToCenter(vec2 force);
		void ApplyLinearImpulseToCenter(vec2 impulse);
	};

	struct Box2DComponent {
		vec2 Size = { 0.5f, 0.5f };
		vec2 Offset = { 0.0f, 0.0f };

		void* RuntimeFixture = nullptr;

		Box2DComponent() = default;
		Box2DComponent(const Box2DComponent&) = default;
	};

	struct CircleColliderComponent {
		vec2 Offset = { 0.0f, 0.0f };
		float Radius = 0.5f;

		void* RuntimeFixture = nullptr;

		CircleColliderComponent() = default;
		CircleColliderComponent(const CircleColliderComponent&) = default;
	};

	template<typename... Component>
	struct ComponentGroup
	{

	};
	using AllComponents =
		ComponentGroup < ParentChildComponent, TransformComponent,
		SpriteRendererComponent, CircleRendererComponent, StringRendererComponent,
		CameraComponent, NativeScriptComponent,
		Rigidbody2DComponent, Box2DComponent, CircleColliderComponent>; 
}