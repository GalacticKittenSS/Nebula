#pragma once

#include "Camera.h"

#include "Nebula/Utils/Time.h"

#include "Nebula/Events/Event.h"
#include "Nebula/Events/Window_Event.h"
#include "Nebula/Events/Mouse_Event.h"


namespace Nebula {
	class OrthographicCameraController {
	public:
		OrthographicCameraController(float aspectRatio = 16.0f / 9.0f, bool rotation = false);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		void OnResize(float width, float height);

		OrthographicCamera& GetCamera() { return m_Camera; }
		const OrthographicCamera& GetCamera() const { return m_Camera; }

		void SetZoomLevel(float level) { m_ZoomLevel = level; CalculateView(); }
		float GetZoomLevel() const { return m_ZoomLevel; }
	private:
		void CalculateView();

		bool OnMouseScrolled(MouseScrolledEvent& event);
		bool OnWindowResized(WindowResizeEvent& event);
	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		OrthographicCamera m_Camera;

		bool m_Rotation;

		vec3 m_CamPosition = { 0.0f, 0.0f, 0.0f };
		float m_CamRotation = 0.0f;

		float m_CamTranslationSpeed = 5.0f, m_CamRotationSpeed = 180.0f;
	};
}