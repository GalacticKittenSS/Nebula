#include <Nebula.h>

class App : public Nebula::Application {
public:
	App() {

	}

	~App() {

	}
};

Nebula::Application* Nebula::createApplication() {
	return new App;
}