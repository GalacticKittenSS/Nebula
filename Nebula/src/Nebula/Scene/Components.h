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
		mat4 Transform{ 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const mat4& transform) : Transform(transform) { }

		operator mat4& () { return Transform; }
		operator const mat4& () const { return Transform; }
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

		std::function<void()> InstantiateFunction;
		std::function<void()> DestroyInstanceFunction;

		std::function<void(ScriptableEntity*)> OnCreateFunction;
		std::function<void(ScriptableEntity*)> OnUpdateFunction;
		std::function<void(ScriptableEntity*)> OnDestroyFunction;

		template<typename T>
		void Bind() {
			InstantiateFunction = [&]() { Instance = new T(); };
			DestroyInstanceFunction = [&]() { delete (T*)Instance; Instance = nullptr; };

			OnCreateFunction = [](ScriptableEntity* instance) { ((T*)instance)->Start(); };
			OnUpdateFunction = [](ScriptableEntity* instance) { ((T*)instance)->Update(); };
			OnDestroyFunction = [](ScriptableEntity* instance) { ((T*)instance)->Destroy(); };
		}
	};
}