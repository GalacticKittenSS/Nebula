#include "Example1.h"

void Example1Layer::Attach() {
	//Set Camera Mat4
	Controller = Nebula::OrthographicCameraController(16.0f / 9.0f, true);
	//SHADERS
	ShaderLib.Load("assets/shaders/Texture.glsl");

	//TEXTURES
	Texture = Nebula::Texture2D::Create("assets/textures/cat.png");
	ShaderLib.Get("Texture")->Bind();
	ShaderLib.Get("Texture")->SetInt("u_Texture", 0);
}

void Example1Layer::Render() {
	//Window Background Colour
	Nebula::RenderCommand::SetClearColour({ 0.1f, 0.1f, 0.1f, 1.0f });
	Nebula::RenderCommand::Clear();

	Nebula::Renderer2D::BeginScene(Controller.GetCamera());

	//Textured Square
	Nebula::Renderer2D::DrawQuad(Nebula::Sprite({ 0, 0 }, { 1, 1 }, 0.0f, Texture));

	//Create 100 Cubes
	for (int y = -10; y < 10; y++) {
		for (int x = -10; x < 10; x++) {
			//Get Position with 0.01 gap
			Nebula::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
			//Render Cube Immediately
			Nebula::Renderer2D::DrawQuad(Nebula::Sprite(pos, { 0.1f, 0.1f }, 0.0f, Texture));
		}
	}

	Nebula::Renderer2D::EndScene();
}

void Example1Layer::Update(Nebula::Timestep ts) {
	Controller.OnUpdate(ts);
}

void Example1Layer::OnEvent(Nebula::Event& e) {
	Controller.OnEvent(e);

	Nebula::Dispatcher dispatcher(e);
	//Calls Client Made Functions on Event
	dispatcher.Dispatch<Nebula::KeyPressedEvent>(BIND_EVENT(Example1Layer::OnKeyPressed));		//Call When Key is Pressed
}

bool Example1Layer::OnKeyPressed(Nebula::KeyPressedEvent& event) {
	NB_TRACE("Key {0} was pressed!", (char)event.GetKeyCode());
	return false;
}