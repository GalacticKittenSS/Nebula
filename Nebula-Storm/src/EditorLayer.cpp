#include "EditorLayer.h"

#include <Nebula/Utils/UI.h>
#include <Nebula/AssetManager/AssetManager.h>

#include <imgui.h>
#include <ImGuizmo.h>

namespace Nebula {
	static glm::vec4 s_CubeVertexPos[] = {
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
	static glm::vec2 s_CubeTexturePos[] = {
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

		m_PlayIcon		= Texture2D::Create("Resources/Icons/Play.png");
		m_SimulateIcon	= Texture2D::Create("Resources/Icons/Simulate.png");
		m_PauseIcon		= Texture2D::Create("Resources/Icons/Pause.png");
		m_StopIcon		= Texture2D::Create("Resources/Icons/Stop.png");
		m_StepIcon		= Texture2D::Create("Resources/Icons/Step.png");
		m_Backdrop		= Texture2D::Create("Resources/Textures/bg.png");

		m_EditorCam = EditorCamera(60.0f, 16.0f / 9.0f, 0.01f, 1000.0f);

		RenderCommand::SetClearColour({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::SetLineWidth(4.0f);

		FontManager::Add(FontFamily("Resources/fonts", "OpenSans"));
		FontManager::Add(FontFamily("Resources/fonts", "Roboto"));
		
		//Initialize Frame Buffer
		FrameBufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INT, FramebufferTextureFormat::Depth };
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		frameBuffer = FrameBuffer::Create(fbSpec);

		//Create New Scene
		NewScene();

		//Open Project on Startup
		auto commandLineArgs = Application::Get().GetSpecification().CommandLineArgs;
		if (commandLineArgs.Count > 1)
			OpenProject(commandLineArgs[1]);
	}

	void EditorLayer::Detach() 
	{
		SaveProject();
	}

	void EditorLayer::Resize() {
		FrameBufferSpecification spec = frameBuffer->GetFrameBufferSpecifications();
		if (m_GameViewSize.x > 0.0f && m_GameViewSize.y > 0.0f
			&& (spec.Width != m_GameViewSize.x || spec.Height != m_GameViewSize.y))
		{
			frameBuffer->Resize((uint32_t)m_GameViewSize.x, (uint32_t)m_GameViewSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)m_GameViewSize.x, (uint32_t)m_GameViewSize.y);
			m_EditorCam.SetViewPortSize(m_GameViewSize.x, m_GameViewSize.y);
		}
	}

	void EditorLayer::GetPixelData() {
		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewPortBounds[0].x;
		my -= m_ViewPortBounds[0].y;
		glm::vec2 viewportSize = m_ViewPortBounds[1] - m_ViewPortBounds[0];
		my = viewportSize.y - my;

		if (mx >= 0 && my >= 0 && mx < viewportSize.x && my < viewportSize.y) {
			int pixelData = frameBuffer->ReadPixel(1, (int)mx, (int)my);
			m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_ActiveScene.get());
		}
	}
	
	void EditorLayer::Update(Timestep ts) {
		NB_PROFILE_FUNCTION();

		Resize();

		switch (m_SceneState) {
			case SceneState::Edit:
				if (!m_UsingGizmo && m_GameViewHovered)
				{
					m_EditorCam.Update();
				
					if (Input::IsMouseButtonPressed(MouseCode::Button0))
						m_TimeCameraMoved += m_EditorCam.HasMoved() ? Time::DeltaTime() : 0.0f;
				}
				
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

		// Frame Counter
		m_Frames++; m_TotalFrames++;
		if (Time::Elapsed() - m_LastTime >= 1.0f) 
		{
			m_LastTime = Time::Elapsed();
			m_LastFrame = m_Frames;
			m_Frames = 0;
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

		if (m_ShowDebug) 
		{
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
				if (ImGui::MenuItem("New Scene", "Ctrl+N"))
					NewScene();

				if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
					SaveScene();

				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
					SaveSceneAs();

				ImGui::Separator();

				if (ImGui::MenuItem("Open Project", "Ctrl+O"))
					OpenProject();

				if (ImGui::MenuItem("Quit"))
					Application::Get().Close();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Settings")) {
				if (ImGui::MenuItem("Show Physics Colliders", NULL, m_ShowColliders))
					m_ShowColliders = !m_ShowColliders;

				if (ImGui::MenuItem("Show Grid", NULL, m_ShowGrid))
					m_ShowGrid = !m_ShowGrid;

				if (ImGui::BeginMenu("Style"))
				{
					if (ImGui::Button("Dark", ImVec2{ 100.0f, 0.0f }))
						Application::Get().GetImGuiLayer()->SetDarkThemeColour();
					
					if (ImGui::Button("Cold", ImVec2{ 100.0f, 0.0f }))
						Application::Get().GetImGuiLayer()->SetColdThemeColour();

					if (ImGui::Button("Light", ImVec2{ 100.0f, 0.0f }))
						Application::Get().GetImGuiLayer()->SetLightThemeColour();

					if (ImGui::Button("Warm", ImVec2{ 100.0f, 0.0f }))
						Application::Get().GetImGuiLayer()->SetWarmThemeColour();

					ImGui::EndMenu();
				}

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

			if (ImGui::BeginMenu("Scripting")) {
				if (ImGui::MenuItem("Reload Assembly"))
					ScriptEngine::ReloadAssembly();

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

	void EditorLayer::OnOverlayRender()  {
		NB_PROFILE_FUNCTION();

		if (m_SceneState == SceneState::Play) 
		{
			Entity camera = m_ActiveScene->GetPrimaryCamera();
			if (!camera)
				return;

			const CameraComponent& cc = camera.GetComponent<CameraComponent>();
			const WorldTransformComponent& wtc = camera.GetComponent<WorldTransformComponent>();
			Renderer2D::BeginScene(cc.Camera, wtc.Transform);
		}
		else 
		{
			Renderer2D::BeginScene(m_EditorCam);

			{
				uint32_t vertexCount = sizeof(s_CubeVertexPos) / sizeof(glm::vec4);
				glm::mat4 transform = glm::translate(m_EditorCam.GetPosition()) * glm::scale(glm::vec3(1000.0f));

				Renderer2D::DrawQuad(vertexCount, s_CubeVertexPos, s_CubeTexturePos, 
					transform, glm::vec4(1.0f), m_Backdrop, 1.0f);
			}

			if (Entity selectedEntity = m_SceneHierarchy.GetSelectedEntity())
				RenderSelectionUI(selectedEntity);
		}

		if (m_ShowColliders)
			RenderColliders();
		
		Renderer2D::EndScene();
	}

	void EditorLayer::RenderColliders() {
		// Calculate z index for translation
		float zIndex = 0.001f;
		glm::vec3 cameraForwardDirection = m_EditorCam.GetForwardDirection();
		glm::vec3 projectionCollider = cameraForwardDirection * glm::vec3(zIndex);

		auto CircleView = m_ActiveScene->GetAllEntitiesWith<WorldTransformComponent, CircleColliderComponent>();
		for (auto entity : CircleView) {
			auto [wtc, cc] = CircleView.get<WorldTransformComponent, CircleColliderComponent>(entity);

			glm::vec3 wTranslation, wRotation, wScale;
			Maths::DecomposeTransform(wtc.Transform, wTranslation, wRotation, wScale);

			glm::vec3 Scale = wScale.x * glm::vec3(cc.Radius * 2.0f);

			glm::mat4 transform = glm::translate(wTranslation) * glm::toMat4(glm::quat(wRotation))
				* glm::translate(glm::vec3(cc.Offset, -projectionCollider.z)) * glm::scale(Scale);
			Renderer2D::DrawCircle(transform, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.05f);
		}

		auto BoxView = m_ActiveScene->GetAllEntitiesWith<WorldTransformComponent, BoxCollider2DComponent>();
		for (auto entity : BoxView) {
			auto [wtc, bc2d] = BoxView.get<WorldTransformComponent, BoxCollider2DComponent>(entity);

			glm::vec3 wTranslation, wRotation, wScale;
			Maths::DecomposeTransform(wtc.Transform, wTranslation, wRotation, wScale);

			glm::vec3 Scale = wScale * glm::vec3(bc2d.Size, 0.0f) * 2.0f;

			glm::mat4 transform = glm::translate(wTranslation) * glm::toMat4(glm::quat(wRotation)) *
				glm::translate(glm::vec3(bc2d.Offset, zIndex)) * glm::scale(Scale);
			Renderer2D::Draw(NB_RECT, transform, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		}
	}

	void EditorLayer::RenderSelectionUI(Entity selectedEntity) 
	{
		if (!selectedEntity.IsEnabled())
			return;

		if (selectedEntity.HasComponent<SpriteRendererComponent>()
			|| selectedEntity.HasComponent<CircleRendererComponent>())
		{
			const WorldTransformComponent& wtc = selectedEntity.GetComponent<WorldTransformComponent>();
			Renderer2D::Draw(NB_RECT, wtc.Transform, glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
		}
		
		for (auto& id : selectedEntity.GetParentChild().ChildrenIDs)
		{
			Entity child = { id, selectedEntity };
			RenderSelectionUI(child);
		}
	}

	void EditorLayer::UI_GameView() {
		UI::ScopedStyleVar padding(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
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

		uint64_t textureID = frameBuffer->GetColourAttachmentRendererID();
		ImGui::Image((void*)textureID, panelSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		if (ImGui::BeginDragDropTarget()) 
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) 
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				LoadScene(path);
			}
			
			ImGui::EndDragDropTarget();
		}

		//Gizmos
		if (m_SceneState != SceneState::Play) 
		{
			m_UsingGizmo = ImGuizmo::IsUsing();

			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			
			ImGuizmo::SetRect(m_ViewPortBounds[0].x, m_ViewPortBounds[0].y, m_ViewPortBounds[1].x - m_ViewPortBounds[0].x, m_ViewPortBounds[1].y - m_ViewPortBounds[0].y);
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

			const glm::mat4& cameraProj = m_EditorCam.GetProjection();
			glm::mat4 cameraView = m_EditorCam.GetViewMatrix();

			if (m_ShowGrid)
				ImGuizmo::DrawGrid(glm::value_ptr(cameraView), glm::value_ptr(cameraProj), glm::value_ptr(glm::mat4(1.0f)), 50.0f);

			Entity selectedEntity = m_SceneHierarchy.GetSelectedEntity();
			if (selectedEntity && m_GizmoType != -1) 
			{
				auto& tc = selectedEntity.GetComponent<TransformComponent>();
				auto& wc = selectedEntity.GetComponent<WorldTransformComponent>();
				glm::mat4 transform = wc.Transform;
				
				bool snap = Input::IsKeyPressed(Key::LeftControl);
				float snapValue = 0.25f;
				if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
					snapValue = 22.5f;

				float snapValues[3] = { snapValue, snapValue, snapValue };

				ImGuizmo::Manipulate(value_ptr(cameraView), value_ptr(cameraProj),
					(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, value_ptr(transform), nullptr, snap ? snapValues : nullptr);

				if (ImGuizmo::IsUsing()) 
				{
					glm::vec3 translation, rotation, scale;
					Maths::DecomposeTransform(transform, translation, rotation, scale);
					
					glm::vec3 wTranslation, wRotation, wScale;
					Maths::DecomposeTransform(wc.Transform, wTranslation, wRotation, wScale);

					tc.Translation += translation - wTranslation;
					tc.Rotation += rotation - wRotation;
					tc.Scale += scale - wScale;

					selectedEntity.UpdateTransform();
				}
			}
		}

		ImGui::End();
	}

	void EditorLayer::UI_Toolbar() {
		UI::ScopedStyleVar padding(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		UI::ScopedStyleVar spacing(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		
		auto& colours = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colours[ImGuiCol_ButtonHovered];
		const auto& buttonActive = colours[ImGuiCol_ButtonActive];
		
		UI::ScopedStyleColor button(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		UI::ScopedStyleColor hovered(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		UI::ScopedStyleColor active(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		float size = ImGui::GetWindowHeight() - 4.0f;

		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
		
		bool editMode = m_SceneState == SceneState::Edit;
		
		if (m_SceneState != SceneState::Simulate)
		{
			Ref<Texture2D> icon = editMode ? m_PlayIcon : m_StopIcon;
			if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0)) {
				if (editMode)
					OnScenePlay();
				else
					OnSceneStop();
			}
			ImGui::SameLine();
		}
		
		if (m_SceneState != SceneState::Play) 
		{
			Ref<Texture2D> icon = editMode ? m_SimulateIcon : m_StopIcon;
			if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0)) {
				if (editMode)
					OnSceneSimulate();
				else
					OnSceneStop();
			}
			ImGui::SameLine();
		}
		
		if (m_SceneState != SceneState::Edit) 
		{
			bool paused = m_ActiveScene->IsPaused();
			Ref<Texture2D> icon = paused ? m_PlayIcon : m_PauseIcon;
			if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0)) {
				m_ActiveScene->SetPaused(!paused);
			}

			if (paused) {
				ImGui::SameLine();
				Ref<Texture2D> icon = m_StepIcon;
				if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f))) {
					m_ActiveScene->Step(10);
				}
			}
		}

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& e) {
		if (m_SceneState != SceneState::Play)
			m_EditorCam.OnEvent(e);

		Dispatcher d(e);
		d.Dispatch<KeyPressedEvent>(BIND_EVENT(EditorLayer::OnKeyPressed));
		d.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT(EditorLayer::OnMouseReleased));
		d.Dispatch<WindowDropEvent>(BIND_EVENT(EditorLayer::OnWindowDrop));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e) {
		if (e.IsRepeat() || m_SceneState == SceneState::Play || !m_GameViewFocus)
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
				OpenProject();
			break;
			
		case KeyCode::D:
			if (Application::Get().GetImGuiLayer()->GetActiveWidgetID() == 0)
			{
				if (control)
					m_SceneHierarchy.SetSelectedEntity(DuplicateEntity());
			}
			break;

		case KeyCode::Backspace:
			if (Application::Get().GetImGuiLayer()->GetActiveWidgetID() == 0)
			{
				if (m_SceneHierarchy.GetSelectedEntity())
				{
					m_EditorScene->DestroyEntity(m_SceneHierarchy.GetSelectedEntity());
					m_SceneHierarchy.SetSelectedEntity({});
				}
			}
			break;

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

	bool EditorLayer::OnMouseReleased(MouseButtonReleasedEvent& e) {
		bool sceneStatePlay = m_SceneState == SceneState::Play;
		bool canSelect = m_TimeCameraMoved <= 0.05f && !ImGuizmo::IsOver() && m_GameViewHovered;

		if (e.GetMouseButton() == Mouse::Button0 && canSelect && !sceneStatePlay)
			m_SceneHierarchy.SetSelectedEntity(m_HoveredEntity);

		m_TimeCameraMoved = 0.0f;

		return false;
	}

	bool EditorLayer::OnWindowDrop(WindowDropEvent& e)
	{
		const auto& filepaths = e.GetPaths();
		
		if (e.GetPaths().size() == 1)
		{
			const std::filesystem::path& path = filepaths[0];
			
			if (path.extension().string() == ".nproj" || path.filename().string() == ".nproj")
			{
				OpenProject(path);
				return true;
			}
			else if (path.extension().string() == ".nebula" || path.filename().string() == ".nebula")
			{
				LoadScene(path);
				return true;
			}
		}

		for (const auto& path : filepaths)
			Project::GetAssetManager()->ImportAsset(path);
		
		return false;
	}

	void EditorLayer::NewProject()
	{
		Project::New();
	}

	void EditorLayer::SaveProject()
	{
		if (Project::GetActive())
			Project::SaveActive(Project::GetProjectFile());
	}

	void EditorLayer::OpenProject()
	{
		std::filesystem::path filepath = FileDialogs::OpenFile("Nebula Project (*.nproj)\0*.nproj\0");
		OpenProject(filepath);
	}

	void EditorLayer::OpenProject(const std::filesystem::path& path)
	{
		SaveProject();

		if (Project::Load(path))
		{
			ScriptEngine::ReloadAssembly();
			m_ContentBrowser.SetContext(Project::GetAssetDirectory());
			
			std::filesystem::path startScenePath = Project::GetAssetFileSystemPath(Project::GetActive()->GetConfig().StartScene);
			LoadScene(startScenePath);
		}
	}

	void EditorLayer::NewScene() {
		if (m_SceneState != SceneState::Edit)
			OnSceneStop();

		ScriptEngine::ClearScriptInstances();
	
		m_EditorScene = CreateRef<Scene>();
		m_EditorScene->OnViewportResize((uint32_t)m_GameViewSize.x, (uint32_t)m_GameViewSize.y);
		m_SceneHierarchy.SetContext(m_EditorScene);
		m_ContentBrowser.SetSceneContext(m_EditorScene);

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

		ScriptEngine::ClearScriptInstances();

		Ref<Scene> empty = CreateRef<Scene>();
		if (SceneSerializer(empty).Deserialize(path.string())) {
			m_EditorScene = empty;
			m_ActiveScene->OnViewportResize((uint32_t)m_GameViewSize.x, (uint32_t)m_GameViewSize.y);
			m_SceneHierarchy.SetContext(m_EditorScene);
			m_ContentBrowser.SetSceneContext(m_EditorScene);

			m_ActiveScene = m_EditorScene;
		}

		m_ScenePath = path.string();
	}

	void EditorLayer::OnScenePlay() 
	{
		if (!m_EditorScene)
			return;

		if (m_SceneState == SceneState::Simulate)
			OnSceneStop();

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnRuntimeStart();

		m_ActiveScene->OnViewportResize((uint32_t)m_GameViewSize.x, (uint32_t)m_GameViewSize.y);
		
		m_SceneHierarchy.SetContext(m_ActiveScene);
		m_ContentBrowser.SetSceneContext(m_ActiveScene);
		m_SceneState = SceneState::Play;
	}

	void EditorLayer::OnSceneStop() 
	{
		NB_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate);

		if (m_SceneState == SceneState::Play)
			m_ActiveScene->OnRuntimeStop();
		else if (m_SceneState == SceneState::Simulate)
			m_ActiveScene->OnSimulationStop();

		m_ActiveScene = m_EditorScene;

		m_SceneHierarchy.SetContext(m_EditorScene);
		m_ContentBrowser.SetSceneContext(m_EditorScene);
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
		m_ContentBrowser.SetSceneContext(m_ActiveScene);
		m_SceneState = SceneState::Simulate;
	}

	Entity EditorLayer::DuplicateEntity() {
		if (m_SceneState != SceneState::Edit)
			return {};
		
		if (Entity entity = m_SceneHierarchy.GetSelectedEntity())
			return m_EditorScene->DuplicateEntity(entity);

		return {};
	}
}