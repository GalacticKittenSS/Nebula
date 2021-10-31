#pragma once

#include "API.h"
#include "Window.h"

#include <GLFW/glfw3.h>

namespace Nebula {
	class NB_API Application {
	public:
		Application();
		~Application();

		void run();
	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
	};

	//Defined In Client
	Application* createApplication();
}