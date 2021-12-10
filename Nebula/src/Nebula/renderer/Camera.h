#pragma once

#include "Nebula/Maths/Maths.h"

namespace Nebula {
	class Camera {
	public:
		Camera() = default;
		Camera(const mat4& projection) : m_ProjectionMatrix(projection) { }
		virtual ~Camera() = default;

		const mat4& GetProjection() const { return m_ProjectionMatrix; }
	protected:
		mat4 m_ProjectionMatrix = mat4(1.0f);
	};

	class OrthographicCamera {
	public:
		OrthographicCamera(float left, float right, float bottom, float top);

		void SetProjection(float left, float right, float bottom, float top);

		const vec3& GetPosition() { return m_Position; }
		void SetPosition(const vec3& position) { m_Position = position; RecalculateViewMatrix(); }

		const float& GetRotation() { return m_Rotation; }
		void SetRotation(const float& rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }

		const mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
	private:
		void RecalculateViewMatrix();
	private:
		mat4 m_ProjectionMatrix;
		mat4 m_ViewMatrix;
		mat4 m_ViewProjectionMatrix;

		vec3 m_Position = { 0, 0, 0 };
		float m_Rotation = 0.0f;
	};
}