#include "EditorLayer.h"

namespace Nebula {
	class CameraController : public ScriptableEntity {
	public:
		void Start() { }

		void Update() {
			auto& position = GetComponent<TransformComponent>().Translation;
			auto& rotation = GetComponent<TransformComponent>().Rotation;

			if (Input::IsKeyPressed(KeyCode::A)) {
				position.x -=  cos(rotation.z) * cos(rotation.y) * speed * Time::DeltaTime();
				position.y -=  sin(rotation.z) * speed * Time::DeltaTime();
				position.z -= -sin(rotation.y) * speed * Time::DeltaTime();
			}

			if (Input::IsKeyPressed(KeyCode::D)) {
				position.x +=  cos(rotation.z) * cos(rotation.y) * speed * Time::DeltaTime();
				position.y +=  sin(rotation.z) * speed * Time::DeltaTime();
				position.z += -sin(rotation.y) * speed * Time::DeltaTime();
			}

			if (Input::IsKeyPressed(KeyCode::W)) {
				position.x += -sin(rotation.y) * speed * Time::DeltaTime();
				position.y +=  sin(rotation.x) * speed * Time::DeltaTime();
				position.z += -cos(rotation.x) * cos(rotation.y) * speed * Time::DeltaTime();
			}

			if (Input::IsKeyPressed(KeyCode::S)) {
				position.x -= -sin(rotation.y) * speed * Time::DeltaTime();
				position.y -=  sin(rotation.x) * speed * Time::DeltaTime();
				position.z -= -cos(rotation.x) * cos(rotation.y) * speed * Time::DeltaTime();
			}

			rotation.y = -(Input::GetMouseX() - Application::Get().GetWindow().GetWidth() / 2) / 360;
			//rotation.x = -(Input::GetMouseY() - Application::Get().GetWindow().GetHeight() / 2) / 360;
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
		
		m_Camera = m_ActiveScene->CreateEntity("Camera");
		
		m_Camera.AddComponent<CameraComponent>();
		m_Camera.AddComponent<NativeScriptComponent>().Bind<CameraController>();

		m_Camera.GetComponent<CameraComponent>().Camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);
		m_Camera.GetComponent<TransformComponent>().Translation.z = 1.0f;

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

		vec4 cubeVertices[] = {
			{ 0, 0, 0, 1 },
			{ 1, 0, 0, 1 },
			{ 1, 1, 0, 1 },
			{ 0, 1, 0, 1 },
			
			{ 1, 0, 1, 1 },
			{ 1, 0, 0, 1 },
			{ 1, 1, 0, 1 },
			{ 1, 1, 1, 1 },

			{ 0, 1, 1, 1 },
			{ 1, 1, 1, 1 },
			{ 1, 1, 0, 1 },
			{ 0, 1, 0, 1 },

			{ 0, 0, 1, 1 },
			{ 1, 0, 1, 1 },
			{ 1, 1, 1, 1 },
			{ 0, 1, 1, 1 },

			{ 0, 0, 0, 1 },
			{ 0, 0, 1, 1 },
			{ 0, 1, 1, 1 },
			{ 0, 1, 0, 1 },

			{ 0, 0, 0, 1 },
			{ 1, 0, 0, 1 },
			{ 1, 0, 1, 1 },
			{ 0, 0, 1, 1 }
		};

		Renderer2D::BeginScene(m_Camera.GetComponent<CameraComponent>().Camera, m_Camera.GetComponent<TransformComponent>().CalculateMatrix());
		Renderer2D::Draw(NB_QUAD, cubeVertices, sizeof(cubeVertices) / sizeof(vec4), translate(vec3{ 0.0f, 0.0f, 0.0f }));
		Renderer2D::EndScene();

		frameBuffer->Unbind();
	}

	void EditorLayer::ImGuiRender() {
		static bool dockspaceOpen = true;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Nebula Storm", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar(3);

		ImGuiID dockspace_id = ImGui::GetID("Nebula Storm");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Quit")) 
					Application::Get().Close();
				
				ImGui::EndMenu();
			}
			
			if (ImGui::BeginMenu("Add")) {
				if (ImGui::MenuItem("Empty")) 
					auto& sprite = m_ActiveScene->CreateEntity("Entity");

				if (ImGui::MenuItem("Sprite")) {
					auto& sprite = m_ActiveScene->CreateEntity("Sprite");
					sprite.AddComponent<SpriteRendererComponent>();
				}

				if (ImGui::MenuItem("Camera")) {
					auto& sprite = m_ActiveScene->CreateEntity("Camera");
					sprite.AddComponent<CameraComponent>();
					sprite.AddComponent<NativeScriptComponent>().Bind<CameraController>();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		m_SceneHierarchy.OnImGuiRender();

		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Game View", nullptr, ImGuiWindowFlags_NoCollapse);
		ImGui::PopStyleVar();

		m_GameViewFocus = ImGui::IsWindowFocused();
		m_GameViewHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->SetBlockEvents(!m_GameViewFocus || !m_GameViewHovered);

		if (m_GameViewFocus)
			m_ActiveScene->Update();

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