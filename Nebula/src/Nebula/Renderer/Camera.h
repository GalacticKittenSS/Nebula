#pragma once

#include "Nebula/Maths/Maths.h"
#include "Nebula/Events/Event.h"
#include "Nebula/Events/Mouse_Event.h"

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

	class EditorCamera : public Camera {
	public:
		EditorCamera() = default;
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		void Update();
		void OnEvent(Event& e);

		inline float GetDistance() const { return m_Distance; }
		inline void SetDistance(float distance) { m_Distance = distance; }

		inline void SetViewPortSize(float width, float height) { m_ViewportWidth = width, m_ViewportHeight = height; UpdateProjection(); }

		const mat4& GetViewMatrix() const { return m_ViewMatrix; }
		mat4 GetViewProjection() const { return m_ProjectionMatrix * m_ViewMatrix; }

		vec3 GetUpDirection() const;
		vec3 GetRightDirection() const;
		vec3 GetForwardDirection() const;
		const vec3& GetPosition() const { return m_Position; }
		quat GetOrientation() const;

		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }
	private:
		void UpdateProjection();
		void UpdateView();

		bool OnMouseScroll(MouseScrolledEvent& e);

		void MousePan(const vec2& delta);
		void MouseRotate(const vec2& delta);
		void MouseZoom(float delta);

		vec3 CalculatePosition() const;

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;
	private:
		float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;

		mat4 m_ViewMatrix;
		vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

		vec2 m_InitialMousePosition = { 0.0f, 0.0f };

		float m_Distance = 10.0f;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;

		float m_ViewportWidth = 1280, m_ViewportHeight = 720;
	};

	class OrthographicCamera : public Camera {
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
		mat4 m_ViewMatrix;
		mat4 m_ViewProjectionMatrix;

		vec3 m_Position = { 0, 0, 0 };
		float m_Rotation = 0.0f;
	};
}
