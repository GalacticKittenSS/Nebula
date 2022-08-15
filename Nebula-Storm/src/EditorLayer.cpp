#include "EditorLayer.h"

namespace Nebula {
	extern const std::filesystem::path s_AssetPath = "SandboxProj/Assets";

	static vec4 s_CubeVertexPos[] = {
		//Front
		{ -0.5f, -0.5f, -0.5f, 1 },
		{  0.5f, -0.5f, -0.5f, 1 },
		{  0.5f,  0.5f, -0.5f, 1 },
		{ -0.5f,  0.5f, -0.5f, 1 },

		//Left
		{  0.5f, -0.5f, -0.5f, 1 },
		{  0.5f, -0.5f,  0.5f, 1 },
		{  0.5f,  0.5f,  0.5f, 1 },
		{  0.5f,  0.5f, -0.5f, 1 },

		//Top
		{ -0.5f,  0.5f, -0.5f, 1 },
		{  0.5f,  0.5f, -0.5f, 1 },
		{  0.5f,  0.5f,  0.5f, 1 },
		{ -0.5f,  0.5f,  0.5f, 1 },

		//Right
		{ -0.5f, -0.5f,  0.5f, 1 },
		{ -0.5f, -0.5f, -0.5f, 1 },
		{ -0.5f,  0.5f, -0.5f, 1 },
		{ -0.5f,  0.5f,  0.5f, 1 },

		//Back
		{ -0.5f, -0.5f,  0.5f, 1 },
		{  0.5f, -0.5f,  0.5f, 1 },
		{  0.5f,  0.5f,  0.5f, 1 },
		{ -0.5f,  0.5f,  0.5f, 1 },

		//Bottom
		{ -0.5f, -0.5f, -0.5f, 1 },
		{  0.5f, -0.5f, -0.5f, 1 },
		{  0.5f, -0.5f,  0.5f, 1 },
		{ -0.5f, -0.5f,  0.5f, 1 }
	};
	static vec2 s_CubeTexturePos[] = {
		//Front
		{ 0.50f, 0.345f },
		{ 0.25f, 0.345f },
		{ 0.25f, 0.66f },
		{ 0.50f, 0.66f },

		//Left
		{ 0.25f, 0.345f },
		{ 0.00f, 0.345f },
		{ 0.00f, 0.66f },
		{ 0.25f, 0.66f },

		//Top
		{ 0.49f, 0.66f },
		{ 0.26f, 0.66f },
		{ 0.26f, 1.00f },
		{ 0.49f, 1.00f },

		//Right
		{ 0.75f, 0.345f },
		{ 0.50f, 0.345f },
		{ 0.50f, 0.66f },
		{ 0.75f, 0.66f },

		//Back
		{ 0.75f, 0.345f },
		{ 1.00f, 0.345f },
		{ 1.00f, 0.66f },
		{ 0.75f, 0.66f },

		//Bottom
		{ 0.499f, 0.345f },
		{ 0.256f, 0.345f },
		{ 0.256f, 0.00f },
		{ 0.499f, 0.00f }
	};

	void EditorLayer::Attach() {
		NB_PROFILE_FUNCTION();

		m_PlayIcon		= Texture2D::Create("Resources/Icons/PlayButton.png");
		m_SimulateIcon	= Texture2D::Create("Resources/Icons/SimulateButton.png");
		m_StopIcon		= Texture2D::Create("Resources/Icons/StopButton.png");
		m_Backdrop		= Texture2D::Create("Resources/Textures/bg.png");

		//Initialize Frame Buffer
		FrameBufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INT, FramebufferTextureFormat::Depth };
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		frameBuffer = FrameBuffer::Create(fbSpec);

		//Create New Scene
		NewScene();

		//Open Scene on Startup
		auto commandLineArgs = Application::Get().GetSpecification().CommandLineArgs;
		if (commandLineArgs.Count > 1)
			LoadScene(commandLineArgs[1]);
		
		m_EditorCam = EditorCamera(60.0f, 16.0f / 9.0f, 0.01f, 1000.0f);

		m_SceneHierarchy.SetContext(m_ActiveScene);
		RenderCommand::SetClearColour({ 0.1f, 0.1f, 0.1f, 1.0f });

		FontManager::Add(new Font("Default",	  "Resources/fonts/OpenSans/Regular.ttf", 86));
		FontManager::Add(new Font("OpenSans",	  "Resources/fonts/OpenSans/Regular.ttf", 86));
		FontManager::Add(new Font("OpenSans - BOLD", "Resources/fonts/OpenSans/Bold.ttf", 86));
		
		RenderCommand::SetLineWidth(4.0f);
	}

	void EditorLayer::Detach() {
		FontManager::Clean();
	}

	void EditorLayer::Resize() {
		FrameBufferSpecification spec = frameBuffer->GetFrameBufferSpecifications();
		if (m_GameViewSize.x > 0.0f && m_GameViewSize.y > 0.0f
			&& (spec.Width != m_GameViewSize.x || spec.Height != m_GameViewSize.y))
		{
			frameBuffer->Resize((uint32_t)m_GameViewSize.x, (uint32_t)m_GameViewSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)m_GameViewSize.x, (uint32_t)m_GameViewSize.y);
			m_EditorCam.SetViewPortSize(m_GameViewSize.x, m_GameViewSize.y);
			OpenSans.SetScale({ m_GameViewSize.x / 32.0f, m_GameViewSize.y / 18.0f });
		}
	}

	void EditorLayer::GetPixelData() {
		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewPortBounds[0].x;
		my -= m_ViewPortBounds[0].y;
		vec2 viewportSize = m_ViewPortBounds[1] - m_ViewPortBounds[0];
		my = viewportSize.y - my;

		if (mx >= 0 && my >= 0 && mx < viewportSize.x && my < viewportSize.y) {
			int pixelData = frameBuffer->ReadPixel(1, (int)mx, (int)my);
			m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_ActiveScene.get());
		}
	}
	
	void EditorLayer::Update(Timestep ts) {
		NB_PROFILE_FUNCTION();

		Resize();

		switch (m_SceneState)
		{
			case SceneState::Edit:
				if (!m_UsingGizmo && m_GameViewHovered)
					m_EditorCam.Update();
				
				m_ActiveScene->UpdateEditor();
				break;
			case SceneState::Simulate:
				if (!m_UsingGizmo && m_GameViewHovered)
					m_EditorCam.Update();

				m_ActiveScene->UpdateSimulation();
				break;
			case SceneState::Play:
				m_ActiveScene->UpdateRuntime();
				break;
		}
	}

	void EditorLayer::Render() {
		NB_PROFILE_FUNCTION();

		frameBuffer->Bind();
		RenderCommand::Clear();

		frameBuffer->ClearAttachment(1, -1);

		switch (m_SceneState) {
			case SceneState::Edit:
				m_ActiveScene->Render(m_EditorCam);
				OnOverlayRender();
				m_ActiveScene->RenderOverlay(m_EditorCam);

				GetPixelData();
				break;

			case SceneState::Simulate:
				m_ActiveScene->Render(m_EditorCam);
				OnOverlayRender();
				m_ActiveScene->RenderOverlay(m_EditorCam);

				GetPixelData();
				break;
			
			case SceneState::Play:
				m_ActiveScene->RenderRuntime();
				OnOverlayRender();
				m_ActiveScene->RenderRuntimeOverlay();
				break;
		}

		frameBuffer->Unbind();
	}

	void EditorLayer::ImGuiRender() {
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
		ImGui::Begin("Nebula Storm", nullptr, window_flags);
		ImGui::PopStyleVar(3);

		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSize = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;

		ImGuiID dockspace_id = ImGui::GetID("Nebula Storm");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

		style.WindowMinSize.x = minWinSize;
		
		UI_MenuBar();

		m_SceneHierarchy.OnImGuiRender();
		m_ContentBrowser.OnImGuiRender();

		UI_GameView();
		UI_Toolbar();

		m_Frames++; m_TotalFrames++;
		if (Time::Elapsed() - m_LastTime >= 1.0f) {
			m_LastTime = Time::Elapsed();
			m_LastFrame = m_Frames;
			m_Frames = 0;
		}

		if (m_ShowDebug) {
			ImGui::Begin("Debug Profiling", &m_ShowDebug);
			ImGui::Text("Time since last Frame: %.3fms", Time::DeltaTime() * 1000.0f);
			ImGui::Text("FPS: %.3f", m_Frames / (Time::Elapsed() - m_LastTime));
			ImGui::Text("");
			ImGui::Text("Time Elapsed: %.3f", Time::Elapsed() - m_TimeSinceReset);
			ImGui::Text("Total Frames: %i", m_TotalFrames);
			ImGui::Text("Average FPS: %.1f", m_TotalFrames / (Time::Elapsed() - m_TimeSinceReset));

			ImGui::SetCursorPosX(ImGui::GetContentRegionAvailWidth() / 2.0f);
			if (ImGui::Button("Reset")) {
				m_TotalFrames = m_LastFrame;
				m_TimeSinceReset = Time::Elapsed();
			}

			ImGui::End();
		}

		Application::Get().GetImGuiLayer()->SetBlockEvents(!m_GameViewFocus && !m_GameViewHovered);
		
		ImGui::End();
	}

	void EditorLayer::UI_MenuBar() {
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New", "Ctrl+N"))
					NewScene();

				if (ImGui::MenuItem("Open...", "Ctrl+O"))
					LoadScene();

				if (ImGui::MenuItem("Save", "Ctrl+S"))
					SaveScene();

				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
					SaveSceneAs();

				if (ImGui::MenuItem("Quit"))
					Application::Get().Close();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Settings")) {
				if (ImGui::MenuItem("Show Physics Colliders", NULL, m_ShowColliders))
					m_ShowColliders = !m_ShowColliders;

				if (ImGui::MenuItem("Show Grid", NULL, m_ShowGrid))
					m_ShowGrid = !m_ShowGrid;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Scene")) {
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
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Windows")) {
				if (ImGui::MenuItem("Debug Profiling"))
					m_ShowDebug = true;

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	}

	void EditorLayer::OnOverlayRender() {
		if (m_SceneState == SceneState::Play) {
			Entity cam = m_ActiveScene->GetPrimaryCamera();
			if (cam)
				Renderer2D::BeginScene(cam.GetComponent<CameraComponent>().Camera, cam.GetComponent<TransformComponent>().CalculateMatrix());
		}
		else {
			Renderer2D::SetBackCulling(false);
			Renderer2D::BeginScene(m_EditorCam);
			Renderer2D::DrawQuad(sizeof(s_CubeVertexPos) / sizeof(vec4), s_CubeVertexPos, s_CubeTexturePos, translate(m_EditorCam.GetPosition()) * scale(vec3(1000.0f)), vec4(1.0f), m_Backdrop, 1.0f);
		}

		if (m_ShowColliders)
			RenderColliders();
		
		Renderer2D::EndScene();
		Renderer2D::SetBackCulling(true);

		if (m_SceneState != SceneState::Play)
		{
			if (Entity selectedEntity = m_SceneHierarchy.GetSelectedEntity())
			{
				Renderer2D::BeginScene(m_EditorCam);
				RenderSelectionUI(selectedEntity);
				Renderer2D::EndScene();
			}
		}
	}

	void EditorLayer::RenderColliders() {
		// Calculate z index for translation
		float zIndex = 0.001f;
		vec3 cameraForwardDirection = m_EditorCam.GetForwardDirection();
		vec3 projectionCollider = cameraForwardDirection * vec3(zIndex);

		auto CircleView = m_ActiveScene->GetAllEntitiesWith<WorldTransformComponent, CircleColliderComponent>();
		for (auto entity : CircleView) {
			auto [wtc, cc] = CircleView.get<WorldTransformComponent, CircleColliderComponent>(entity);

			vec3 wTranslation, wRotation, wScale;
			DecomposeTransform(wtc.Transform, wTranslation, wRotation, wScale);

			vec3 Scale = wScale.x * vec3(cc.Radius * 2.0f);

			mat4 transform = translate(wTranslation) * toMat4(quat(wRotation))
				* translate(vec3(cc.Offset, -projectionCollider.z)) * scale(Scale);
			Renderer2D::DrawCircle(transform, vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.05f);
		}

		auto BoxView = m_ActiveScene->GetAllEntitiesWith<WorldTransformComponent, BoxCollider2DComponent>();
		for (auto entity : BoxView) {
			auto [wtc, bc2d] = BoxView.get<WorldTransformComponent, BoxCollider2DComponent>(entity);

			vec3 wTranslation, wRotation, wScale;
			DecomposeTransform(wtc.Transform, wTranslation, wRotation, wScale);

			vec3 Scale = wScale * vec3(bc2d.Size) * 2.0f;

			mat4 transform = translate(wTranslation) * toMat4(quat(wRotation)) *
				translate(vec3(bc2d.Offset, zIndex)) * scale(Scale);
			Renderer2D::Draw(NB_RECT, transform, vec4(0.0f, 1.0f, 0.0f, 1.0f));
		}
	}

	void EditorLayer::RenderSelectionUI(Entity selectedEntity) {
		const WorldTransformComponent& transform = selectedEntity.GetComponent<WorldTransformComponent>();

		if (selectedEntity.HasComponent<SpriteRendererComponent>() || selectedEntity.HasComponent<CircleRendererComponent>())
			Renderer2D::Draw(NB_RECT, transform.Transform * translate(vec3(0.0f, 0.0f, 0.01f)), vec4(1.0f, 0.5f, 0.0f, 1.0f));

		for (auto& child : selectedEntity.GetParentChild().ChildrenIDs)
			RenderSelectionUI({ child, selectedEntity });
	}

	void EditorLayer::UI_GameView() {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Game View", nullptr, ImGuiWindowFlags_NoCollapse);

		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewPortBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewPortBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		m_GameViewFocus = ImGui::IsWindowFocused();
		m_GameViewHovered = ImGui::IsWindowHovered();

		ImVec2 panelSize = ImGui::GetContentRegionAvail();
		m_GameViewSize = { panelSize.x, panelSize.y };

		uint32_t textureID = frameBuffer->GetColourAttachmentRendererID();
		ImGui::Image((void*)textureID, panelSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
				const wchar_t* path = (const wchar_t*)payload->Data;
				LoadScene(s_AssetPath / path);
			}
			ImGui::EndDragDropTarget();
		}

		m_UsingGizmo = ImGuizmo::IsUsing();

		//Gizmos
		if (m_SceneState != SceneState::Play) {
			Entity selectedEntity = m_SceneHierarchy.GetSelectedEntity();
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			
			ImGuizmo::SetRect(m_ViewPortBounds[0].x, m_ViewPortBounds[0].y, m_ViewPortBounds[1].x - m_ViewPortBounds[0].x, m_ViewPortBounds[1].y - m_ViewPortBounds[0].y);
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

			const mat4& cameraProj = m_EditorCam.GetProjection();
			mat4 cameraView = m_EditorCam.GetViewMatrix();

			if (m_ShowGrid)
				ImGuizmo::DrawGrid(value_ptr(cameraView), value_ptr(cameraProj), value_ptr(mat4(1.0f)), 50.0f);

			if (selectedEntity && m_GizmoType != -1) {
				auto& tc = selectedEntity.GetComponent<TransformComponent>();
				auto& wc = selectedEntity.GetComponent<WorldTransformComponent>();
				mat4 transform = wc.Transform;
				
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
					
					vec3 wTranslation, wRotation, wScale;
					DecomposeTransform(wc.Transform, wTranslation, wRotation, wScale);

					tc.Translation += translation - wTranslation;
					tc.Rotation += rotation - wRotation;
					tc.Scale += scale - wScale;

					UpdateChildrenAndTransform(selectedEntity);
				}
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void EditorLayer::UI_Toolbar() {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colours = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colours[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		const auto& buttonActive = colours[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		float size = ImGui::GetWindowHeight() - 4.0f;

		{
			bool editorsimulate = m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate;
			Ref<Texture2D> icon = editorsimulate ? m_PlayIcon : m_StopIcon;
			ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0)) {
				if (editorsimulate)
					OnScenePlay();
				else
					OnSceneStop();
			}
		}
		ImGui::SameLine();
		{
			bool editorplay = m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play;
			Ref<Texture2D> icon = editorplay ? m_SimulateIcon : m_StopIcon;
			//ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0)) {
				if (editorplay)
					OnSceneSimulate();
				else
					OnSceneStop();
			}
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& e) {
		if (m_SceneState != SceneState::Play)
			m_EditorCam.OnEvent(e);

		Dispatcher d(e);
		d.Dispatch<KeyPressedEvent>(BIND_EVENT(EditorLayer::OnKeyPressed));
		d.Dispatch<MouseButtonPressedEvent>(BIND_EVENT(EditorLayer::OnMousePressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e) {
		if (e.IsRepeat() > 0 || m_SceneState == SceneState::Play || !m_GameViewFocus)
			return false;

		bool control = Input::IsKeyPressed(KeyCode::LeftControl) || Input::IsKeyPressed(KeyCode::RightControl);
		bool shift = Input::IsKeyPressed(KeyCode::LeftShift) || Input::IsKeyPressed(KeyCode::RightShift);
		switch (e.GetKeyCode())
		{
		//Scene Saving/Loading
		case KeyCode::S:
			if (control && shift)
				SaveSceneAs();
			else if (control)
				SaveScene();
			break;
		case KeyCode::N:
			if (control)
				NewScene();
			break;
		case KeyCode::O:
			if (control)
				LoadScene();
			break;

		case KeyCode::D:
			if (control && (m_GameViewFocus || m_SceneHierarchy.IsFocused()))
				DuplicateEntity();
			break;

		case KeyCode::Backspace:
			if (m_SceneHierarchy.GetSelectedEntity() && (m_GameViewFocus || m_SceneHierarchy.IsFocused())) {
				m_EditorScene->DestroyEntity(m_SceneHierarchy.GetSelectedEntity());

				m_SceneHierarchy.SetSelectedEntity({});
			}

		//Gizmos
		case KeyCode::Q:
			if (!m_UsingGizmo && (!m_GameViewFocus || !m_SceneHierarchy.IsFocused()))
				m_GizmoType = -1;
			break;
		case KeyCode::W:
			if (!m_UsingGizmo && (!m_GameViewFocus || !m_SceneHierarchy.IsFocused()))
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			break;
		case KeyCode::E:
			if (!m_UsingGizmo && (!m_GameViewFocus || !m_SceneHierarchy.IsFocused()))
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			break;
		case KeyCode::R:
			if (!m_UsingGizmo && (!m_GameViewFocus || !m_SceneHierarchy.IsFocused()))
				m_GizmoType = ImGuizmo::OPERATION::SCALE;
			break;
		}

		return true;
	}

	bool EditorLayer::OnMousePressed(MouseButtonPressedEvent& e) {
		if (e.GetMouseButton() == Mouse::Button0 && !ImGuizmo::IsOver() && m_GameViewHovered && m_SceneState != SceneState::Play)
			m_SceneHierarchy.SetSelectedEntity(m_HoveredEntity);

		return false;
	}

	void EditorLayer::NewScene() {
		if (m_SceneState != SceneState::Edit)
			OnSceneStop();
	
		m_EditorScene = CreateRef<Scene>();
		m_EditorScene->OnViewportResize((uint32_t)m_GameViewSize.x, (uint32_t)m_GameViewSize.y);
		m_SceneHierarchy.SetContext(m_EditorScene);

		m_ActiveScene = m_EditorScene;
		
		m_HoveredEntity = { };
		m_ScenePath = "";
	}

	void EditorLayer::SaveSceneAs() {
		std::string filepath = FileDialogs::SaveFile("Nebula Scene (*.nebula)\0*.nebula\0");
		
		if (!filepath.empty())
			SceneSerializer(m_ActiveScene).Serialize(filepath);

		m_ScenePath = filepath;
	}

	void EditorLayer::SaveScene() {
		if (!m_ScenePath.empty())
			SceneSerializer(m_ActiveScene).Serialize(m_ScenePath);
		else
			SaveSceneAs();
	}

	void EditorLayer::LoadScene() {
		std::string filepath = FileDialogs::OpenFile("Nebula Scene (*.nebula)\0*.nebula\0");
		
		if (!filepath.empty())
			LoadScene(filepath);
	}

	void EditorLayer::LoadScene(const std::filesystem::path& path) {
		if (path.extension().string() != ".nebula") {
			NB_WARN("Could not load {0} - not a scene file", path.filename().string());
			return;
		}

		if (m_SceneState != SceneState::Edit)
			OnSceneStop();

		Ref<Scene> empty = CreateRef<Scene>();
		if (SceneSerializer(empty).Deserialize(path.string())) {
			m_EditorScene = empty;
			m_ActiveScene->OnViewportResize((uint32_t)m_GameViewSize.x, (uint32_t)m_GameViewSize.y);
			m_SceneHierarchy.SetContext(m_EditorScene);

			m_ActiveScene = m_EditorScene;
		}

		m_ScenePath = path.string();
	}

	void EditorLayer::OnScenePlay() {
		if (!m_EditorScene)
			return;

		if (m_SceneState == SceneState::Simulate)
			OnSceneStop();

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnRuntimeStart();

		m_ActiveScene->OnViewportResize((uint32_t)m_GameViewSize.x, (uint32_t)m_GameViewSize.y);
		
		m_SceneHierarchy.SetContext(m_ActiveScene);
		m_SceneState = SceneState::Play;
	}

	void EditorLayer::OnSceneStop() {
		NB_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate);

		if (m_SceneState == SceneState::Play)
			m_ActiveScene->OnRuntimeStop();
		else if (m_SceneState == SceneState::Simulate)
			m_ActiveScene->OnSimulationStop();

		m_ActiveScene = m_EditorScene;

		auto& entities = m_ActiveScene->GetAllEntitiesWith<StringRendererComponent>();
		for (auto& ent : entities) {
			auto& src = Entity{ ent, m_ActiveScene.get() }.GetComponent<StringRendererComponent>();
			src.InitiateFont();
		}

		m_SceneHierarchy.SetContext(m_EditorScene);
		m_SceneState = SceneState::Edit;
	}

	void EditorLayer::OnSceneSimulate() {
		if (!m_EditorScene)
			return;

		if (m_SceneState == SceneState::Play)
			OnSceneStop();

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnSimulationStart();

		m_ActiveScene->OnViewportResize((uint32_t)m_GameViewSize.x, (uint32_t)m_GameViewSize.y);

		m_SceneHierarchy.SetContext(m_ActiveScene);
		m_SceneState = SceneState::Simulate;
	}

	void EditorLayer::DuplicateEntity() {
		if (m_SceneState != SceneState::Edit)
			return;
		
		Entity selectedEnt = m_SceneHierarchy.GetSelectedEntity();
		if (selectedEnt)
			m_EditorScene->DuplicateEntity(selectedEnt);
	}
}