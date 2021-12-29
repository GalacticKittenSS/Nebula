#pragma once

#include <Nebula.h>

#include "Panels/Scene_Hierarchy.h"
#include "Panels/Content_Browser.h"

namespace Nebula {
	class EditorLayer : public Layer {
	public:
		EditorLayer();
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

		void NewScene();
		void SaveScene();
		void SaveSceneAs();
		void LoadScene();
		void LoadScene(const std::filesystem::path& path);

		void OnScenePlay();
		void OnSceneStop();

		void DuplicateEntity();

		//Panels
		void UI_MenuBar();
		void UI_Toolbar();
		void UI_GameView();
	private:
		bool m_GameViewFocus = false, m_GameViewHovered = false;
		bool m_UsingGizmo = false;
		bool m_ShowColliders = false;
		
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
			Edit = 0, Play = 1
		};

		SceneState m_SceneState = SceneState::Edit;

		//Panels
		SceneHierarchyPanel m_SceneHierarchy;
		ContentBrowserPanel m_ContentBrowser;

		//Editor Resources
		Ref<Texture2D> m_PlayIcon, m_StopIcon;
	};
}