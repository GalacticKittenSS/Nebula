#include <Nebula.h>
#include <Nebula_EntryPoint.h>

#define Flappy

#ifdef E1
	#include "Example1/Example1.h"
#elif defined Sandbox
	#include "Sandbox/Sandbox2D.h"
#elif defined Flappy
	#include "Flappy Bird/Game.h"
#endif

//Application Class (Calls Layers)
class App : public Nebula::Application {
public:
	App() {
		//Add Layer to List for Rendering and Startup
#if defined E1
		PushLayer(new Example1Layer());
#elif defined Sandbox
		PushLayer(new Sandbox2D());
#elif defined Flappy
		PushLayer(new Game());
#endif
	}

	~App() { }
};

//Returns App to Nebula Engine on Startup
Nebula::Application* Nebula::createApplication() {
	return new App;
}