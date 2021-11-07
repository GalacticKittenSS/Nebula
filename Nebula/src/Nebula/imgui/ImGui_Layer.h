#pragma once

#include "Nebula/Layer.h"
#include "Nebula/Application.h"

#include "imgui/imgui.h"
#include "Platform/OpenGl/ImGui_Renderer.h"
#include <GLFW/glfw3.h>

#include "Nebula/events/Mouse_Event.h"
#include "Nebula/events/Key_Event.h"
#include "Nebula/events/Window_Event.h"

namespace Nebula {
	class NB_API ImGuiLayer : public Layer {
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void Update() override;

		void OnAttach() override;
		void OnDetach() override;
		void OnEvent(Event& e) override;
	private:
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& e);
		bool OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& e);
		bool OnMouseMovedEvent(MouseMovedEvent& e);
		bool OnMouseScrolledEvent(MouseScrolledEvent& e);
		
		bool OnKeyPressedEvent(KeyPressedEvent& e);
		bool OnKeyReleasedEvent(KeyReleasedEvent& e);
		bool OnKeyTypedEvent(KeyTypedEvent& e);
		
		bool OnWindowResizeEvent(WindowResizeEvent& e);
	private:
		float m_Time = 0.0f;
	};
}