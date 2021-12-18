#include <Nebula.h>
#include <Nebula_EntryPoint.h>

#include "EditorLayer.h"

namespace Nebula {
	class NebulaStorm : public Application {
	public:
		NebulaStorm(ApplicationCommandLineArgs args): Application("Nebula Storm", args) {
			PushLayer(new EditorLayer());
		}

		~NebulaStorm() { }
	};

	Application* CreateApplication(ApplicationCommandLineArgs args) {
		return new NebulaStorm(args);
	}
}