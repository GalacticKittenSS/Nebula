#include "nbpch.h"
#include "Camera_Controller.h"

#include "Nebula/Core/Input.h"

namespace Nebula {
	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation): 
		m_AspectRatio(aspectRatio),
		m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio* m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel),
		m_Rotation(rotation) {
	}


	void OrthographicCameraController::CalculateView() {
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	}

	void OrthographicCameraController::OnUpdate(Timestep ts) {
		NB_PROFILE_FUNCTION();

		if (Input::IsKeyPressed(NB_W)) {
			m_CamPosition.x += -glm::sin(glm::radians(m_CamRotation)) * m_CamTranslationSpeed * ts;
			m_CamPosition.y +=  glm::cos(glm::radians(m_CamRotation)) * m_CamTranslationSpeed * ts;
		}
		else if (Input::IsKeyPressed(NB_S)) {
			m_CamPosition.x -= -sin(glm::radians(m_CamRotation)) * m_CamTranslationSpeed * ts;
			m_CamPosition.y -=  cos(glm::radians(m_CamRotation)) * m_CamTranslationSpeed * ts;
		}
		if (Input::IsKeyPressed(NB_A)) {
			m_CamPosition.x -= glm::cos(glm::radians(m_CamRotation)) * m_CamTranslationSpeed * ts;
			m_CamPosition.y -= glm::sin(glm::radians(m_CamRotation)) * m_CamTranslationSpeed * ts;
		}
		else if (Input::IsKeyPressed(NB_D)) {
			m_CamPosition.x += glm::cos(glm::radians(m_CamRotation)) * m_CamTranslationSpeed * ts;
			m_CamPosition.y += glm::sin(glm::radians(m_CamRotation)) * m_CamTranslationSpeed * ts;
		}

		if (m_Rotation) {
			if (Input::IsKeyPressed(NB_Q))
				m_CamRotation += m_CamRotationSpeed * ts;

			if (Input::IsKeyPressed(NB_E))
				m_CamRotation -= m_CamRotationSpeed * ts;

			if (m_CamRotation > 180.0f)
				m_CamRotation -= 360.0f;
			else if (m_CamRotation <= -180.0f)
				m_CamRotation += 360.0f;
			
			m_Camera.SetRotation(m_CamRotation);
		}

		m_Camera.SetPosition(m_CamPosition);
	}

	void OrthographicCameraController::OnResize(float width, float height) {
		m_AspectRatio = width / height;
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	}

	void OrthographicCameraController::OnEvent(Event& e) {
		NB_PROFILE_FUNCTION();

		Dispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT(OrthographicCameraController::OnWindowResized));
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& event) {
		NB_PROFILE_FUNCTION();

		m_ZoomLevel -= event.GetOffsetY() * 0.25f;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		CalculateView();
		m_CamTranslationSpeed = 2.0f * m_ZoomLevel;

		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& event) {
		NB_PROFILE_FUNCTION();

		OnResize((float)event.GetWidth(), (float)event.GetHeight());
		CalculateView();

		return false;
	}
}
