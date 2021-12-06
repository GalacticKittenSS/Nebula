#include "Sandbox2D.h"

Sandbox2D::Sandbox2D() : Layer("Sandbox"), Controller(16.0f / 9.0f) { }

void Sandbox2D::Attach() {
	NB_PROFILE_FUNCTION();

	Texture = Nebula::Texture2D::Create("assets/textures/cat.png");
	timer = Nebula::Timer();
}

void Sandbox2D::Detach() {

}

void Sandbox2D::Update(Nebula::Timestep ts) {
	NB_PROFILE_FUNCTION();
	
	Controller.OnUpdate(ts);
}

void Sandbox2D::Render() {
	NB_PROFILE_FUNCTION();

	//Window Background Colour
	Nebula::RenderCommand::SetClearColour({ 0.1f, 0.1f, 0.1f, 1.0f });
	//Clear the Screen of the Previous Frame
	Nebula::RenderCommand::Clear();

	Nebula::Renderer2D::ResetStats();

	//Begin Scene with Camera
	Nebula::Renderer2D::BeginScene(Controller.GetCamera());
	for (float y = -5.0f; y < 5.0f; y += 0.5f) {
		for (float x = -5.0f; x < 5.0f; x += 0.5f) {
			Nebula::vec3 colour = { (x + 5.0f) / 10.0f, 0.4f, (y + 0.5f) / 10.0f };
			Nebula::Renderer2D::DrawQuad(Nebula::Sprite({ x, y }, { 0.45f, 0.45f }, 0.0f, { colour.r, colour.g, colour.b, 0.2f }));
		}
	}
	Nebula::Renderer2D::EndScene();

	Nebula::Renderer2D::BeginScene(Controller.GetCamera());
	Nebula::Renderer2D::DrawQuad(Nebula::Sprite({
		 Nebula::cos(timer.elapsed()) + Nebula::sin(timer.elapsed()),
		-Nebula::sin(timer.elapsed()) + Nebula::cos(timer.elapsed())
			 }, { 1.0f, 1.0f }, 0.0f, { 0.2f, 0.4f, 0.7f, 1.0f }));
	Nebula::Renderer2D::DrawQuad(Nebula::Sprite({-5.0f, 0.2f }, { 2.0f, 4.0f }, 10.0f, { 1.0f, 0.1f, 0.2f, 1.0f }));
	Nebula::Renderer2D::DrawQuad(Nebula::Sprite({ 2.0f, 2.0f }, { 2.0f, 2.0f }, 150.0f, { 0.2f, 0.4f, 0.7f, 1.0f }));
	Nebula::Renderer2D::DrawQuad(Nebula::Sprite({ 0.0f, 0.0f }, { 100.0f, 100.0f }, 0.0f, Nebula::Texture2D::Create("assets/textures/checkerboard.png")), 10.0f);
	Nebula::Renderer2D::EndScene();
}

void Sandbox2D::ImGuiRender() {
	auto stats = Nebula::Renderer2D::GetStats();
	
	ImGui::Begin("Renderer2D Stats");
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Quads: %d", stats.QuadCount);
	ImGui::Text("Triangles: %d", stats.TriCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::End();
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