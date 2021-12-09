#include <Nebula.h>
#include <Nebula_EntryPoint.h>

#define Sandbox

#if defined E1
	#include "Example1/Example1.h"
#elif defined Sandbox
	#include "Sandbox/Sandbox2D.h"
#elif defined Flappy
	#include "Flappy Bird/Game.h"
#elif defined Tile
	#include "Tilemaps/Game.h"
#endif

class BG : public Nebula::Layer {
public:
	void Render() override {
		Nebula::RenderCommand::Clear();

		Nebula::Renderer2D::BeginScene(cam);
		Nebula::Renderer2D::DrawQuad(sprite);
		Nebula::Renderer2D::EndScene();
	}
private:
	Nebula::OrthographicCamera cam = Nebula::OrthographicCamera(-0.8f, 0.8f, -0.45f, 0.45f);
	Nebula::Sprite sprite = Nebula::Sprite({ 0.0f, 0.0f, -0.9f }, { 1.6f, 0.9f }, 0.0f, Nebula::Texture2D::Create("assets/textures/Backdrop.jpg"));
};

//Application Class (Calls Layers)
class App : public Nebula::Application {
public:
	App() {
		//Add Layer to List for Rendering and Startup
		PushLayer(new BG());

#if defined E1
		PushLayer(new Example1Layer());
#elif defined Sandbox
		PushLayer(new Sandbox2D());
#else
		PushLayer(new Game());
#endif
	}

	~App() { }
};

//Returns App to Nebula Engine on Startup
Nebula::Application* Nebula::createApplication() {
	return new App;
}