#include "EditorLayer.h"

namespace Nebula {
	class CameraController : public ScriptableEntity {
	public:
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
			rotation.x = -(Input::GetMouseY() - Application::Get().GetWindow().GetHeight() / 2) / 360;
		}
	private:
		float speed = 10.0f;
	};

	EditorLayer::EditorLayer() : Layer("Editor") { }

	void EditorLayer::Attach() {
		NB_PROFILE_FUNCTION();

		FrameBufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INT, FramebufferTextureFormat::Depth };
		fbSpec.Width = 1280;
		fbSpec.Height = 720;

		frameBuffer = FrameBuffer::Create(fbSpec);
		timer = Timer();

		m_SceneHierarchy.SetContext(m_ActiveScene);

		m_EditorCam = EditorCamera(60.0f, 16.0f / 9.0f, 0.01f, 1000.0f);

		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_GameViewSize.x, (uint32_t)m_GameViewSize.y);
		m_SceneHierarchy.SetContext(m_ActiveScene);

		SceneSerializer(m_ActiveScene).Deserialize("assets/scenes/PinkCube.nebula");
	}

	void EditorLayer::Detach() {

	}

	void EditorLayer::Update(Timestep ts) {
		NB_PROFILE_FUNCTION();


		FrameBufferSpecification spec = frameBuffer->GetFrameBufferSpecifications();
		if (m_GameViewSize.x > 0.0f && m_GameViewSize.y > 0.0f && (spec.Width != m_GameViewSize.x || spec.Height != m_GameViewSize.y)) {
			frameBuffer->Resize((uint32_t)m_GameViewSize.x, (uint32_t)m_GameViewSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)m_GameViewSize.x, (uint32_t)m_GameViewSize.y);
			m_EditorCam.SetViewPortSize(m_GameViewSize.x, m_GameViewSize.y);
		}

		if (!m_UsingGizmo && m_GameViewHovered) {
			m_EditorCam.Update();
		}
	}

	void EditorLayer::Render() {
		NB_PROFILE_FUNCTION();

		frameBuffer->Bind();

		RenderCommand::SetClearColour({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();

		frameBuffer->ClearAttachment(1, -1);

		m_ActiveScene->UpdateEditor(m_EditorCam);

		vec2 viewportSize = m_ViewPortBounds[1] - m_ViewPortBounds[0];

		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewPortBounds[0].x;
		my -= m_ViewPortBounds[0].y;
		my = viewportSize.y - my;

		if (m_GameViewHovered) {
			int pixelData = frameBuffer->ReadPixel(1, mx, my);
			m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_ActiveScene.get());
		}

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

		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSize = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;

		ImGuiID dockspace_id = ImGui::GetID("Nebula Storm");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

		style.WindowMinSize.x = minWinSize;
		
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New", "Ctrl+N"))
					NewScene();

				if (ImGui::MenuItem("Open...", "Ctrl+O"))
					LoadScene();
				
				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
					SaveSceneAs();
				
				if (ImGui::MenuItem("Quit")) 
					Application::Get().Close();
				
				ImGui::EndMenu();
			}
			
			if (ImGui::BeginMenu("Create Entity")) {
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

		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewPortBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewPortBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		m_GameViewFocus = ImGui::IsWindowFocused();
		m_GameViewHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->SetBlockEvents(!m_GameViewFocus && !m_GameViewHovered);

		if (m_GameViewFocus)
			m_ActiveScene->UpdateRuntime();

		ImVec2 panelSize = ImGui::GetContentRegionAvail();
		m_GameViewSize = { panelSize.x, panelSize.y };

		uint32_t textureID = frameBuffer->GetColourAttachmentRendererID();
		ImGui::Image((void*)textureID, panelSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		
		m_UsingGizmo = ImGuizmo::IsOver();

		//Gizmos
		Entity selectedEntity = m_SceneHierarchy.GetSelectedEntity();
		if (selectedEntity && m_GizmoType != -1) {
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(m_ViewPortBounds[0].x, m_ViewPortBounds[0].y, m_ViewPortBounds[1].x - m_ViewPortBounds[0].x, m_ViewPortBounds[1].y - m_ViewPortBounds[0].y);

			float windowWidth = (float)ImGui::GetWindowWidth();
			float windowHeight = (float)ImGui::GetWindowHeight();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

			const mat4& cameraProj = m_EditorCam.GetProjection();
			mat4 cameraView = m_EditorCam.GetViewMatrix();

			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			mat4 transform = tc.CalculateMatrix();

			bool snap = Input::IsKeyPressed(Key::LeftControl);
			float snapValue = 0.25f;
			if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 22.5f;

			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate(value_ptr(cameraView), value_ptr(cameraProj), 
				(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, value_ptr(transform), nullptr, snap ? snapValues : nullptr);


			if (ImGuizmo::IsUsing()) {
				vec3 translation, rotation, scale;
				DecomposeTransform(transform, translation, rotation, scale);

				vec3 deltaRotation = rotation - tc.Rotation;
				tc.Translation = translation;
				tc.Rotation += deltaRotation;
				tc.Scale = scale;
			}
		}

		ImGui::End();
		
		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& e) {
		m_EditorCam.OnEvent(e);

		Dispatcher d(e);
		d.Dispatch<KeyPressedEvent>(BIND_EVENT(EditorLayer::OnKeyPressed));
		d.Dispatch<MouseButtonPressedEvent>(BIND_EVENT(EditorLayer::OnMousePressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e) {
		if (e.GetRepeatCount() > 0)
			return false;

		bool control = Input::IsKeyPressed(KeyCode::LeftControl) || Input::IsKeyPressed(KeyCode::RightControl);
		bool shift = Input::IsKeyPressed(KeyCode::LeftShift) || Input::IsKeyPressed(KeyCode::RightShift);
		switch (e.GetKeyCode())
		{
		//Scene Saving/Loading
		case KeyCode::S:
			if (control && shift)
				SaveSceneAs();
			break;
		case KeyCode::N:
			if (control)
				NewScene();
			break;
		case KeyCode::O:
			if (control)
				LoadScene();
			break;

		//Gizmos
		case KeyCode::Q:
			if (!m_UsingGizmo)
				m_GizmoType = -1;
			break;
		case KeyCode::W:
			if (!m_UsingGizmo)
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			break;
		case KeyCode::E:
			if (!m_UsingGizmo)
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			break;
		case KeyCode::R:
			if (!m_UsingGizmo)
				m_GizmoType = ImGuizmo::OPERATION::SCALE;
			break;
		}

		return true;
	}


	bool EditorLayer::OnMousePressed(MouseButtonPressedEvent& e) {
		if (e.GetMouseButton() == Mouse::Button0 && !m_UsingGizmo && m_GameViewHovered)
			m_SceneHierarchy.SetSelectedEntity(m_HoveredEntity);

		return false;
	}

	void EditorLayer::NewScene() {
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_GameViewSize.x, (uint32_t)m_GameViewSize.y);
		m_SceneHierarchy.SetContext(m_ActiveScene);

		m_HoveredEntity = { };
	}

	void EditorLayer::SaveSceneAs() {
		std::string filepath = FileDialogs::SaveFile("Nebula Scene (*.nebula)\0*.nebula\0");
		std::string ending = ".nebula";

		if (!equal(ending.rbegin(), ending.rend(), filepath.rbegin()))
			filepath += ending;

		if (!filepath.empty())
			SceneSerializer(m_ActiveScene).Serialize(filepath);
	}

	void EditorLayer::LoadScene() {
		std::string filepath = FileDialogs::OpenFile("Nebula Scene (*.nebula)\0*.nebula\0");

		if (!filepath.empty()) {
			NewScene();
			SceneSerializer(m_ActiveScene).Deserialize(filepath);
		}
	}
}