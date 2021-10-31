#pragma once

#include "API.h"

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