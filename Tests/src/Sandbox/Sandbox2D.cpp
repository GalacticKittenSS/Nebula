#include "Sandbox2D.h"

#define SPRITESIZE 0.5f
#define SPRITEAXIS SPRITESIZE * 10.0f

Sandbox2D::Sandbox2D() : Layer("Sandbox"), Controller(16.0f / 9.0f) { }

void Sandbox2D::Attach() {
	NB_PROFILE_FUNCTION();

	Nebula::FrameBufferSpecification fbSpec;
	fbSpec.Width = 1280;
	fbSpec.Height = 720;

	frameBuffer = Nebula::FrameBuffer::Create(fbSpec);
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
	frameBuffer->Bind();

	//Window Background Colour
	Nebula::RenderCommand::SetClearColour({ 0.1f, 0.1f, 0.1f, 1.0f });
	//Clear the Screen of the Previous Frame
	Nebula::RenderCommand::Clear();

	//Begin Scene with Camera
	Nebula::Renderer2D::BeginScene(Controller.GetCamera());
	Nebula::Renderer2D::DrawQuad(Nebula::Sprite({ Nebula::cos(timer.elapsed()) + Nebula::sin(timer.elapsed()), -Nebula::sin(timer.elapsed()) + Nebula::cos(timer.elapsed()) }, { 1.0f, 1.0f } ));

	Nebula::Renderer2D::DrawQuad(Nebula::Sprite({
		 Nebula::cos(timer.elapsed()) + Nebula::sin(timer.elapsed()),
		-Nebula::sin(timer.elapsed()) + Nebula::cos(timer.elapsed())
			 }, { 1.0f, 1.0f }, 0.0f, { 0.2f, 0.4f, 0.7f, 1.0f }));
	
	Nebula::Renderer2D::DrawQuad(Nebula::Sprite({-5.0f, 0.2f }, { 2.0f, 3.0f },  0.0f, { 1.0f, 0.1f, 0.2f, 1.0f }));
	Nebula::Renderer2D::DrawQuad(Nebula::Sprite({ 2.0f, 2.0f }, { 2.0f, 2.0f }, 45.0f, { 0.2f, 0.4f, 0.7f, 1.0f }));
	
	
	for (float y = -SPRITEAXIS; y < SPRITEAXIS; y += SPRITESIZE) {
		for (float x = -SPRITEAXIS; x < SPRITEAXIS; x += SPRITESIZE) {
			Nebula::vec3 colour = { (x + SPRITEAXIS) / (SPRITEAXIS * 2), 0.4f, (y + SPRITEAXIS) / (SPRITEAXIS * 2) };
			Nebula::Renderer2D::DrawQuad(Nebula::Sprite({ x, y, -0.1f }, { SPRITESIZE - 0.05f, SPRITESIZE - 0.05f }, 0.0f, { colour.r, colour.g, colour.b, 0.5f }));
			Nebula::Renderer2D::DrawTriangle(Nebula::Sprite({ x, y, -0.1f }, { SPRITESIZE - 0.05f, SPRITESIZE - 0.05f }, 0.0f, { colour.g, colour.b, colour.r, 0.5f }));
		}
	}

	Nebula::Renderer2D::DrawQuad(Nebula::Sprite({ 0.0f, 0.0f, -0.9f }, { 100.0f, 100.0f }, 0.0f, Nebula::Texture2D::Create("assets/textures/checkerboard.png")), 10.0f);
	Nebula::Renderer2D::DrawQuad(Nebula::Sprite({ 0, 0 }, { 1, 1 }, 0.0, Nebula::Texture2D::Create("assets/textures/cat.png")));
	
	Nebula::Renderer2D::EndScene();
	frameBuffer->Unbind();
}

void Sandbox2D::ImGuiRender() {
	static bool dockspaceOpen = true;
	static bool opt_fullscreen_persistant = true;
	bool opt_fullscreen = opt_fullscreen_persistant;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
	ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit")) Nebula::Application::Get().Close();
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	ImGui::Begin("Settings");
		
	uint32_t textureID = frameBuffer->GetColourAttachmentRendererID();
	ImGui::Image((void*)textureID, ImVec2{ 1280, 720 });
	ImGui::End();


	ImGui::End();
}

void Sandbox2D::OnEvent(Nebula::Event& e) {
	Controller.OnEvent(e);

	Nebula::Dispatcher d(e);
	d.Dispatch<Nebula::KeyPressedEvent>(BIND_EVENT(Sandbox2D::OnKeyPressed));
}

bool Sandbox2D::OnKeyPressed(Nebula::KeyPressedEvent& e) {
	return false;
}