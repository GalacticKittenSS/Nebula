#include <Nebula.h>
#include <Nebula_EntryPoint.h>

//Example 1
//#define E1	
//Example 2 (Sandbox 2D)
#define Sandbox 

#ifdef E1
	#include "Example1.h"
#elif defined Sandbox
	#include "Sandbox2D.h"
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
#endif
	}

	~App() { }
};

//Returns App to Nebula Engine on Startup
Nebula::Application* Nebula::createApplication() {
	return new App;
}