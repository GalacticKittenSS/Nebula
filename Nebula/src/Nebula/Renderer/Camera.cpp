#include "nbpch.h"
#include "Camera.h"

#include "Nebula/Core/Input.h"

namespace Nebula {
	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip) {

	}

	void EditorCamera::UpdateProjection() {
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_ProjectionMatrix = perspective(radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
	}
	
	void EditorCamera::UpdateView() {
		// m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
		m_Position = CalculatePosition();

		quat orientation = GetOrientation();
		m_ViewMatrix = translate(mat4(1.0f), m_Position) * toMat4(orientation);
		m_ViewMatrix = inverse(m_ViewMatrix);
	}
	
	std::pair<float, float> EditorCamera::PanSpeed() const {
		float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::RotationSpeed() const {
		return 0.8f;
	}

	float EditorCamera::ZoomSpeed() const {
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}


	void EditorCamera::Update() {
		const vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
		vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
		m_InitialMousePosition = mouse;

		if (Input::IsMouseButtonPressed(Mouse::ButtonMiddle))
			MousePan(delta);
		else if (Input::IsMouseButtonPressed(Mouse::ButtonLeft))
			MouseRotate(delta);
		else if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
			MouseZoom(delta.y);
		
		UpdateView();
	}

	void EditorCamera::OnEvent(Event& e) {
		Dispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT(EditorCamera::OnMouseScroll));
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e) {
		float delta = e.GetOffsetY() * 0.1f;
		MouseZoom(delta);
		UpdateView();
		return false;
	}


	void EditorCamera::MousePan(const vec2& delta) {
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate(const vec2& delta) {
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed();
		m_Pitch += delta.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta) {
		m_Distance -= delta * ZoomSpeed();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetForwardDirection();
			m_Distance = 1.0f;
		}
	}

	vec3 EditorCamera::GetUpDirection() const {
		return rotate(GetOrientation(), vec3(0.0f, 1.0f, 0.0f));
	}

	vec3 EditorCamera::GetRightDirection() const {
		return rotate(GetOrientation(), vec3(1.0f, 0.0f, 0.0f));
	}

	vec3 EditorCamera::GetForwardDirection() const {
		return rotate(GetOrientation(), vec3(0.0f, 0.0f, -1.0f));
	}
	
	vec3 EditorCamera::CalculatePosition() const {
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}
	
	quat EditorCamera::GetOrientation() const {
		return quat(vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top):
		m_ViewMatrix(1.0f) {
		NB_PROFILE_FUNCTION();

		SetProjection(left, right, bottom, top);
	}

	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top) {
		NB_PROFILE_FUNCTION();

		m_ProjectionMatrix = ortho(left, right, bottom, top, -1.0f, 1.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::RecalculateViewMatrix() {
		NB_PROFILE_FUNCTION();

		mat4 transform = translate(m_Position)
			* rotate(radians(m_Rotation), vec3(0, 0, 1));

		m_ViewMatrix = inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
}
