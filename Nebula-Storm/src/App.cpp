#include <Nebula.h>
#include <Nebula_EntryPoint.h>

#include "EditorLayer.h"

namespace Nebula {
	class NebulaStorm : public Application {
	public:
		NebulaStorm(): Application("Nebula Storm") {
			PushLayer(new EditorLayer());
		}

		~NebulaStorm() { }
	};

	Application* createApplication() {
		return new NebulaStorm;
	}
}