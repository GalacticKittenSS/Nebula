#pragma once

#include "Nebula/Maths/Maths.h"
#include "Nebula/Events/Event.h"
#include "Nebula/Events/Mouse_Event.h"

namespace Nebula {
	class Camera {
	public:
		Camera() = default;
		Camera(const glm::mat4& projection) : m_ProjectionMatrix(projection) { }
		virtual ~Camera() = default;

		const glm::mat4& GetProjection() const { return m_ProjectionMatrix; }
	protected:
		glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
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

		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		glm::mat4 GetViewProjection() const { return m_ProjectionMatrix * m_ViewMatrix; }

		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;
		const glm::vec3& GetPosition() const { return m_Position; }
		glm::quat GetOrientation() const;

		bool HasMoved() { return m_HasMoved; }

		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }
	private:
		void UpdateProjection();
		void UpdateView();

		bool OnMouseScroll(MouseScrolledEvent& e);

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		glm::vec3 CalculatePosition() const;

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;
	private:
		bool m_HasMoved = false;

		float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;

		glm::mat4 m_ViewMatrix;
		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

		glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };

		float m_Distance = 10.0f;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;

		float m_ViewportWidth = 1280, m_ViewportHeight = 720;
	};

	class OrthographicCamera : public Camera {
	public:
		OrthographicCamera(float left, float right, float bottom, float top);

		void SetProjection(float left, float right, float bottom, float top);

		const glm::vec3& GetPosition() { return m_Position; }
		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }

		const float& GetRotation() { return m_Rotation; }
		void SetRotation(const float& rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }

		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
	private:
		void RecalculateViewMatrix();
	private:
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position = { 0, 0, 0 };
		float m_Rotation = 0.0f;
	};
}
