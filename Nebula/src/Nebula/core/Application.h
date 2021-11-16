#pragma once

#include "Window.h"

#include "Layer_Stack.h"
#include "Nebula/imgui/ImGui_Layer.h"

#include "Nebula/Events/Event.h"
#include "Nebula/Events/Window_Event.h"

#include "Time.h"

int main(int argc, char** argv);

namespace Nebula {
	class NB_API Application {
	public:
		Application();
		virtual ~Application();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopOverlay(Layer* overlay);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }
	private:
		void run();
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
	private:
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGui;
		bool m_Running = true, m_Minimized = false;
		LayerStack m_LayerStack;

		Timestep m_Timestep;
		float m_LastFrameTime;

		static Application* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	//Defined In Client
	Application* createApplication();
}