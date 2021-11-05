#pragma once

#include "Window.h"
#include "Nebula/Layer_Stack.h"
#include "Nebula/events/Window_Event.h"

#include <GLFW/glfw3.h>

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
	private:
		bool OnWindowClose(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		LayerStack m_LayerStack;
	};

	//Defined In Client
	Application* createApplication();
}