#pragma once

#include "Nebula/Maths/Maths.h"

#include "Nebula/Renderer/Camera.h"
#include "Entity.h"
#include "Scene_Camera.h"

namespace Nebula {
	struct TagComponent {
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : Tag(tag) { }
	};

	struct TransformComponent {
		vec3 Translation =	{ 0.0f, 0.0f, 0.0f };
		vec3 Rotation =		{ 0.0f, 0.0f, 0.0f };
		vec3 Scale =		{ 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const vec3& translation) : Translation(translation) { }

		operator mat4 () { return CalculateMatrix(); }

		mat4 CalculateMatrix() {
			mat4 rotation = rotate(Rotation.x, { 1, 0, 0 })
				* rotate(Rotation.y, { 0, 1, 0 })
				* rotate(Rotation.z, { 0, 0, 1 });

			return translate(Translation) * rotation * scale(Scale);
		}
	};

	struct SpriteRendererComponent {
		vec4 Colour{ 1.0f, 1.0f, 1.0f, 1.0f };

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const vec4& colour) : Colour(colour) { }
	};

	struct CameraComponent {
		SceneCamera Camera;
		bool Primary = true;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

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
}