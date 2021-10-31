#pragma once

#include "API.h"
#include "Nebula/events/Window_Event.h"
#include "Log.h"

namespace Nebula {
	class NB_API Application {
	public:
		Application();
		~Application();

		void run();
	};

	//Defined In Client
	Application* createApplication();
}