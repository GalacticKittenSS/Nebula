#pragma once

#include <Nebula.h>

#include "Panels/Scene_Hierarchy.h"

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

		void OnEvent(Nebula::Event& e) override;
	private:
		bool OnKeyPressed(KeyPressedEvent& e);

		void NewScene();
		void SaveSceneAs();
		void LoadScene();
	private:
		bool m_GameViewFocus = false, m_GameViewHovered = false;
		vec2 m_GameViewSize = { 1280.0f, 720.0f };

		Ref<Scene> m_ActiveScene;
		Ref<FrameBuffer> frameBuffer;
		Timer timer;

		Entity m_Camera;

		int m_GizmoType = -1;

		//Panels
		SceneHierarchyPanel m_SceneHierarchy;
	};
}