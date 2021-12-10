#pragma once

#include "Nebula/Maths/Maths.h"

#include "Nebula/Renderer/Camera.h"
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
}