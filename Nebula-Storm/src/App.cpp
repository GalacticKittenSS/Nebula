#include <Nebula.h>
#include <Nebula_EntryPoint.h>

#include "EditorLayer.h"

namespace Nebula {
	Application* CreateApplication(ApplicationCommandLineArgs args) {
		ApplicationSpecification spec;
		spec.Name = "Nebula Storm";
		spec.CommandLineArgs = args;

		Application* app = new Application(spec);
		app->PushLayer(new EditorLayer());

		return app;
	}
}