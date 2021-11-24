#include "Sandbox2D.h"

Sandbox2D::Sandbox2D() : Layer("Sandbox"), Controller(16.0f / 9.0f) { }

void Sandbox2D::Attach() {
	NB_PROFILE_FUNCTION();

	Texture = Nebula::Texture2D::Create("assets/textures/cat.png");
}

void Sandbox2D::Detach() {

}

void Sandbox2D::Update(Nebula::Timestep ts) {
	NB_PROFILE_FUNCTION();

	Controller.OnUpdate(ts);
}

void Sandbox2D::Render() {
	//Window Background Colour
	Nebula::RenderCommand::SetClearColour({ 0.1f, 0.1f, 0.1f, 1.0f });
	//Clear the Screen of the Previous Frame
	Nebula::RenderCommand::Clear();

	NB_PROFILE_FUNCTION();
		
	//Begin Scene with Camera
	Nebula::Renderer2D::BeginScene(Controller.GetCamera());

	Nebula::Sprite quad({ 0.0f, 0.0f, 0.1f }, { 10.0f, 10.0f });
	quad.texture = Texture;

	Nebula::Renderer2D::DrawQuad(quad, 10.0f);
	Nebula::Renderer2D::DrawQuad(Nebula::Sprite({ 0.0f, 0.0f }, { 10.0f, 10.0f }, 0.0f, glm::vec4(squareColour, 1.0f)));
	Nebula::Renderer2D::DrawQuad(Nebula::Sprite({ 0.8f, 0.2f, 0.2f }, { 1.0f, 1.0f }, 0.0f, { 0.8f, 0.3f, 0.2f, 1.0f }));

	Nebula::Renderer2D::EndScene();
}

void Sandbox2D::ImGuiRender() {

}

void Sandbox2D::OnEvent(Nebula::Event& e) {
	Controller.OnEvent(e);

	Nebula::Dispatcher d(e);
	d.Dispatch<Nebula::KeyPressedEvent>(BIND_EVENT(Sandbox2D::OnKeyPressed));
}

bool Sandbox2D::OnKeyPressed(Nebula::KeyPressedEvent& e) {
	//NB_TRACE("Key {0} was pressed!", (char)e.GetKeyCode());
	return false;
}