#include "EditorLayer.h"

namespace Nebula {
	class CameraController : public ScriptableEntity {
	public:
		void Start() { }

		void Update() {
			auto& position = GetComponent<TransformComponent>().Translation;
			auto& rotation = GetComponent<TransformComponent>().Rotation;

			if (Input::IsKeyPressed(KeyCode::A))
				position.x -= speed * Time::DeltaTime();
			if (Input::IsKeyPressed(KeyCode::D))
				position.x += speed * Time::DeltaTime();
			if (Input::IsKeyPressed(KeyCode::W))
				position.y += speed * Time::DeltaTime();
			if (Input::IsKeyPressed(KeyCode::S))
				position.y -= speed * Time::DeltaTime();

			rotation.y = -(Input::GetMouseX() - Application::Get().GetWindow().GetWidth() / 2) / 360;
			rotation.x = -(Input::GetMouseY() - Application::Get().GetWindow().GetHeight() / 2) / 360;
		}

		void Destroy() {

		}
	private:
		float speed = 10.0f;
	};

	EditorLayer::EditorLayer() : Layer("Editor") { }

	void EditorLayer::Attach() {
		NB_PROFILE_FUNCTION();

		FrameBufferSpecification fbSpec;
		fbSpec.Width = 1280;
		fbSpec.Height = 720;

		frameBuffer = FrameBuffer::Create(fbSpec);
		timer = Timer();

		m_ActiveScene = CreateRef<Scene>();
		
		auto& Camera = m_ActiveScene->CreateEntity("Camera");
		auto& vertex2 = m_ActiveScene->CreateEntity("Vertex B");
		auto& vertex  = m_ActiveScene->CreateEntity("Vertex A");

		Camera.AddComponent<CameraComponent>();
		vertex.AddComponent<SpriteRendererComponent>(vec4{ 0.0f, 1.0f, 0.0f, 1.0f });
		vertex2.AddComponent<SpriteRendererComponent>(vec4{ 1.0f, 0.0f, 0.0f, 1.0f });

		Camera.AddComponent<NativeScriptComponent>().Bind<CameraController>();

		m_SceneHierarchy.SetContext(m_ActiveScene);
	}

	void EditorLayer::Detach() {

	}

	void EditorLayer::Update(Timestep ts) {
		NB_PROFILE_FUNCTION();

		FrameBufferSpecification spec = frameBuffer->GetFrameBufferSpecifications();
		if (m_GameViewSize.x > 0.0f && m_GameViewSize.y > 0.0f && (spec.Width != m_GameViewSize.x || spec.Height != m_GameViewSize.y)) {
			frameBuffer->Resize((uint32_t)m_GameViewSize.x, (uint32_t)m_GameViewSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)m_GameViewSize.x, (uint32_t)m_GameViewSize.y);
		}
	}

	void EditorLayer::Render() {
		NB_PROFILE_FUNCTION();

		frameBuffer->Bind();

		RenderCommand::SetClearColour({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();

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
				if (ImGui::MenuItem("Quit")) Application::Get().Close();
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		m_SceneHierarchy.OnImGuiRender();
		
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

	void EditorLayer::OnEvent(Event& e) {

	}
}