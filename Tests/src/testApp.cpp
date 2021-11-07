#include <Nebula.h>

class ExampleLayer : public Nebula::Layer {
public:
	ExampleLayer(): Layer("Example") { }

	void Update() override { }

	void OnEvent(Nebula::Event& event) override {
		//CL_INFO("{0}", event);
	}

	void Render() override {
		ImGui::Begin("Test");
		ImGui::Text("Hello World!");
		ImGui::End();
	}
};

class App : public Nebula::Application {
public:
	App() {
		//PushOverlay(new ExampleLayer());
	}

	~App() {

	}
};

Nebula::Application* Nebula::createApplication() {
	return new App;
}