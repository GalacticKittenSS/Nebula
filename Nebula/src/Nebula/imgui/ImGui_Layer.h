#pragma once

#include "Nebula/Layer.h"

#include "Nebula/Application.h"
#include "imgui.h"
#include "Platform/OpenGl/ImGui_Renderer.h"
#include <GLFW/glfw3.h>

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
		float m_Time = 0.0f;
	};
}