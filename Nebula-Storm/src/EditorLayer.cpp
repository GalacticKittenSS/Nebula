#include "EditorLayer.h"

#define SPRITESIZE 0.5f
#define SPRITEAXIS SPRITESIZE * 10.0f

namespace Nebula {
	EditorLayer::EditorLayer() : Layer("Editor"), Controller(16.0f / 9.0f) { }

	void EditorLayer::Attach() {
		NB_PROFILE_FUNCTION();

		Nebula::FrameBufferSpecification fbSpec;
		fbSpec.Width = 1280;
		fbSpec.Height = 720;

		frameBuffer = Nebula::FrameBuffer::Create(fbSpec);
		timer = Nebula::Timer();

		m_ActiveScene = CreateRef<Scene>();
		square = m_ActiveScene->CreateEntity("Square");
		Camera = m_ActiveScene->CreateEntity("Camera");
		Camera2 = m_ActiveScene->CreateEntity("Camera2");

		square.AddComponent<SpriteRendererComponent>(vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
		Camera.AddComponent<CameraComponent>();
		Camera2.AddComponent<CameraComponent>();
	}

	void EditorLayer::Detach() {

	}

	void EditorLayer::Update(Nebula::Timestep ts) {
		NB_PROFILE_FUNCTION();

		FrameBufferSpecification spec = frameBuffer->GetFrameBufferSpecifications();
		if (m_GameViewSize.x > 0.0f && m_GameViewSize.y > 0.0f && (spec.Width != m_GameViewSize.x || spec.Height != m_GameViewSize.y)) {
			frameBuffer->Resize((uint32_t)m_GameViewSize.x, (uint32_t)m_GameViewSize.y);
			Controller.OnResize(m_GameViewSize.x, m_GameViewSize.y);

			m_ActiveScene->OnViewportResize((uint32_t)m_GameViewSize.x, (uint32_t)m_GameViewSize.y);
		}

		if (m_GameViewFocus)
			Controller.OnUpdate(ts);
	}

	void EditorLayer::Render() {
		NB_PROFILE_FUNCTION();

		frameBuffer->Bind();

		Nebula::RenderCommand::SetClearColour({ 0.1f, 0.1f, 0.1f, 1.0f });
		Nebula::RenderCommand::Clear();

		m_ActiveScene->Render();
		
		frameBuffer->Unbind();
	}

	void EditorLayer::ImGuiRender() {
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
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Quit")) Nebula::Application::Get().Close();
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::Begin("Settings");
		if (square) {
			ImGui::Separator();
			ImGui::Text("%s", square.GetComponent<TagComponent>().Tag.c_str());
			
			auto& squareColor = square.GetComponent<SpriteRendererComponent>().Colour;
			ImGui::ColorEdit4("Square Colour: ", value_ptr(squareColor));
			
			ImGui::Separator();
		}

		ImGui::DragFloat3("Camera Transform", value_ptr(Camera.GetComponent<TransformComponent>().Transform[3]));

		if (ImGui::Checkbox("Camera A", &m_Primary)) {
			Camera.GetComponent<CameraComponent>().Primary = m_Primary;
			Camera2.GetComponent<CameraComponent>().Primary = !m_Primary;
		}


		auto& cam = Camera2.GetComponent<CameraComponent>().Camera;
		float size = cam.GetOrthographicSize();

		if (ImGui::DragFloat("Camera 2 Ortho Size", &size))
			cam.SetOrthographicSize(size);

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Game View");
		ImGui::PopStyleVar();

		m_GameViewFocus = ImGui::IsWindowFocused();
		m_GameViewHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->SetBlockEvents(!m_GameViewFocus || !m_GameViewHovered);

		ImVec2 panelSize = ImGui::GetContentRegionAvail();
		m_GameViewSize = { panelSize.x, panelSize.y };

		uint32_t textureID = frameBuffer->GetColourAttachmentRendererID();
		ImGui::Image((void*)textureID, panelSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		ImGui::End();


		ImGui::End();
	}

	void EditorLayer::OnEvent(Nebula::Event& e) {
		Controller.OnEvent(e);
	}
}