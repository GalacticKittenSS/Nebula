#pragma once

#include <Nebula.h>

#include "Panels/Scene_Hierarchy.h"
#include "Panels/Content_Browser.h"

namespace Nebula {
	class EditorLayer : public Layer {
	public:
		EditorLayer() : Layer("Editor") { }
		~EditorLayer() = default;

		void Attach() override;
		void Detach() override;

		void Update(Nebula::Timestep ts) override;
		void Render() override;
	
		void ImGuiRender() override;
		void OnOverlayRender();

		void OnEvent(Nebula::Event& e) override;
	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseReleased(MouseButtonReleasedEvent& e);
		bool OnWindowDrop(WindowDropEvent& e);

		void RenderSelectionUI(Entity selectedEntity);
		
		void Resize();
		void GetPixelData();

		void NewProject();
		void SaveProject();
		void OpenProject();
		void OpenProject(const std::filesystem::path& path);
		
		void NewScene();
		void SaveScene();
		void SaveSceneAs();
		void LoadScene();
		void LoadScene(const std::filesystem::path& path);

		void OnScenePlay();
		void OnSceneStop();
		void OnSceneSimulate();
		void OnScenePause();
		
		Entity DuplicateEntity();

		//Panels
		void UI_MenuBar();
		void UI_Toolbar();
		void UI_GameView();
	private:
		bool m_GameViewFocus = false, m_GameViewHovered = false;
		bool m_UsingGizmo = false;
		bool m_ShowGrid = false;

		bool m_ShowDebug = false;
		uint32_t m_Frames = 0;
		uint32_t m_LastFrame = 0;
		uint32_t m_TotalFrames = 0;
		float m_LastTime;
		float m_TimeSinceReset = 0;
		
		glm::vec2 m_GameViewSize = { 1280.0f, 720.0f };
		glm::vec2 m_ViewPortBounds[2];

		Ref<Scene> m_ActiveScene, m_EditorScene;
		Ref<SceneRenderer> m_SceneRenderer;
		std::string m_ScenePath;
		Timer timer;

		Entity m_Camera;
		Entity m_HoveredEntity;
		
		EditorCamera m_EditorCam;
		float m_TimeCameraMoved = 0.0f;

		int m_GizmoType = -1;

		enum class SceneState {
			Edit = 0, Simulate = 1, Play = 2
		};

		SceneState m_SceneState = SceneState::Edit;

		//Panels
		Ref<SceneHierarchyPanel> m_SceneHierarchy;
		Ref<ContentBrowserPanel> m_ContentBrowser;

		//Editor Resources
		Ref<Texture2D> m_PlayIcon, m_SimulateIcon, 
			m_StopIcon, m_PauseIcon, m_StepIcon, 
			m_Backdrop;
	};
}