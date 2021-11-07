#include <Nebula.h>

class ExampleLayer : public Nebula::Layer {
public:
	ExampleLayer(): Layer("Example") { }

	void Update() override { }

	void OnEvent(Nebula::Event& event) override {
		CL_INFO("{0}", event);
	}
};

class App : public Nebula::Application {
public:
	App() {
		//PushLayer(new ExampleLayer());
	}

	~App() {

	}
};

Nebula::Application* Nebula::createApplication() {
	return new App;
}