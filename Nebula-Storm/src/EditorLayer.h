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
		bool OnMousePressed(MouseButtonPressedEvent& e);

		void RenderSelectionUI(Entity selectedEntity);
		void RenderColliders();

		void Resize();
		void GetPixelData();

		void NewScene();
		void SaveScene();
		void SaveSceneAs();
		void LoadScene();
		void LoadScene(const std::filesystem::path& path);

		void OnScenePlay();
		void OnSceneStop();
		void OnSceneSimulate();
		void OnScenePause();
		
		void DuplicateEntity();

		//Panels
		void UI_MenuBar();
		void UI_Toolbar();
		void UI_GameView();
	private:
		bool m_GameViewFocus = false, m_GameViewHovered = false;
		bool m_UsingGizmo = false;
		bool m_ShowColliders = false;
		bool m_ShowGrid = false;

		bool m_ShowDebug = false;
		uint32_t m_Frames = 0;
		uint32_t m_LastFrame = 0;
		uint32_t m_TotalFrames = 0;
		float m_LastTime;
		float m_TimeSinceReset = 0;
		
		vec2 m_GameViewSize = { 1280.0f, 720.0f };
		vec2 m_ViewPortBounds[2];

		Ref<Scene> m_ActiveScene, m_EditorScene;
		std::string m_ScenePath;
		Ref<FrameBuffer> frameBuffer;
		Timer timer;

		Entity m_Camera;
		Entity m_HoveredEntity;
		
		EditorCamera m_EditorCam;

		int m_GizmoType = -1;

		enum class SceneState {
			Edit = 0, Simulate = 1, Play = 2
		};

		SceneState m_SceneState = SceneState::Edit;

		//Panels
		SceneHierarchyPanel m_SceneHierarchy;
		ContentBrowserPanel m_ContentBrowser;

		//Editor Resources
		Ref<Texture2D> m_PlayIcon, m_SimulateIcon, 
			m_StopIcon, m_PauseIcon, m_StepIcon, 
			m_Backdrop;

		Font OpenSans = Font("OpenSans", "Resources/fonts/OpenSans/Regular.ttf", 64);
	};
}