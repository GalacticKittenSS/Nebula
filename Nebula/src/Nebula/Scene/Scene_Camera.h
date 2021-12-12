#pragma once

#include "Nebula/Renderer/Camera.h"

namespace Nebula {
	class SceneCamera : public Camera {
	public:
		enum class ProjectionType { Perspective = 0, Orthographic = 1 };
	public:
		SceneCamera();
		~SceneCamera() = default;

		void SetOrthographic(float size, float nearClip, float farClip);
		void SetPerspective(float fov, float nearClip, float farClip);

		void SetViewPortSize(uint32_t width, uint32_t height);

		ProjectionType GetProjectionType() const { return m_ProjectionType; }
		void SetProjectionType(ProjectionType type) { m_ProjectionType = type; RecalculateProjectionMatrix(); }

		float GetOrthographicSize() const { return m_OrthographicSize; }
		void SetOrthographicSize(float size) { m_OrthographicSize = size; RecalculateProjectionMatrix(); }
		float GetOrthoNearClip() const { return m_OrthographicNear; }
		void SetOrthoNearClip(float clip) { m_OrthographicNear = clip; RecalculateProjectionMatrix(); }
		float GetOrthoFarClip() const { return m_OrthographicFar; }
		void SetOrthoFarClip(float clip) { m_OrthographicFar = clip; RecalculateProjectionMatrix(); }

		float GetPerspectiveFOV() const { return m_PerspectiveFOV; }
		void SetPerspectiveFOV(float fov) { m_PerspectiveFOV = fov; RecalculateProjectionMatrix(); }
		float GetPerspectiveNearClip() const { return m_PerspectiveNear; }
		void SetPerspectiveNearClip(float clip) { m_PerspectiveNear = clip; RecalculateProjectionMatrix(); }
		float GetPerspectiveFarClip() const { return m_PerspectiveFar; }
		void SetPerspectiveFarClip(float clip) { m_PerspectiveFar = clip; RecalculateProjectionMatrix(); }
	private:
		void RecalculateProjectionMatrix();
	private:
		ProjectionType m_ProjectionType = ProjectionType::Orthographic;

		float m_OrthographicSize = 10.0f;
		float m_OrthographicNear = -1.0f, m_OrthographicFar = 1.0f;

		float m_PerspectiveFOV = radians(45.0f);
		float m_PerspectiveNear = 0.01f, m_PerspectiveFar = 1000.0f;

		float m_AspectRatio = 16.0f / 9.0f;
	};
}