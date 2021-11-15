#include "Sandbox2D.h"

Sandbox2D::Sandbox2D() : Layer("Sandbox"), Controller(16.0f / 9.0f) { }

void Sandbox2D::Attach() {
	Texture = Nebula::Texture2D::Create("assets/textures/rgba.png");
}

void Sandbox2D::Detach() {

}

void Sandbox2D::Update(Nebula::Timestep ts) {
	Controller.OnUpdate(ts); 
}

void Sandbox2D::Render() {
	//Window Background Colour
	Nebula::RenderCommand::SetClearColour({ 0.1f, 0.1f, 0.1f, 1.0f });
	//Clear the Screen of the Previous Frame
	Nebula::RenderCommand::Clear();

	//Begin Scene with Camera
	Nebula::Renderer2D::BeginScene(Controller.GetCamera());

	Nebula::Renderer2D::DrawQuad({ 0.0f, 0.0f, 0.1f }, { 1.0f, 1.0f }, Texture);
	Nebula::Renderer2D::DrawQuad({ 0.0f, 0.0f }, { 10.0f, 10.0f }, glm::vec4(squareColour, 1.0f));
	Nebula::Renderer2D::DrawQuad({ 0.8f, 0.2f }, { 1.0f, 1.0f }, { 0.8f, 0.3f, 0.2f, 1.0f });

	Nebula::Renderer2D::EndScene();
}

void Sandbox2D::ImGuiRender() {
	ImGui::Begin("Settings");
	ImGui::ColorEdit3("Square", glm::value_ptr(squareColour));
	ImGui::End();
}

void Sandbox2D::OnEvent(Nebula::Event& e) {
	Controller.OnEvent(e);
}