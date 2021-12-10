#pragma once

#include "Nebula/Renderer/Camera.h"

namespace Nebula {
	class SceneCamera : public Camera {
	public:
		SceneCamera();
		~SceneCamera() = default;

		void SetOrthographic(float size, float nearClip, float farClip);

		void SetViewPortSize(uint32_t width, uint32_t height);

		float GetOrthographicSize() const { return m_OrthographicSize; }
		void SetOrthographicSize(float size) { m_OrthographicSize = size; RecalculateProjectionMatrix(); }
	private:
		void RecalculateProjectionMatrix();
	private:
		float m_OrthographicSize = 10.0f;
		float m_OrthographicNear = -1.0f, m_OrthographicFar = 1.0f;

		float m_AspectRatio = 16.0f / 9.0f;
	};
}