#include "Application.h"

namespace Nebula {
	Application::Application() {

	}

	Application::~Application() {

	}

	void Application::run() {
		WindowResizeEvent e(1280, 720);
		NB_TRACE(e);

		while (true);
	}
}