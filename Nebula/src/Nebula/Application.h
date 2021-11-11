#pragma once

#include "Window.h"

#include "Layer_Stack.h"
#include "imgui/ImGui_Layer.h"

#include "events/Event.h"
#include "events/Window_Event.h"

#include "core/Timestep.h"

namespace Nebula {
	class NB_API Application {
	public:
		Application();
		~Application();

		void run();
		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopOverlay(Layer* overlay);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);
	private:
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGui;
		bool m_Running = true;
		LayerStack m_LayerStack;

		Timestep m_Timestep;
		float m_LastFrameTime;

		static Application* s_Instance;
	};

	//Defined In Client
	Application* createApplication();
}