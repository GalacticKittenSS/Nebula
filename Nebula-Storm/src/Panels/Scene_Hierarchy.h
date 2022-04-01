#pragma once

#include <Nebula.h>

namespace Nebula {
	class SceneHierarchyPanel {
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void SetContext(const Ref<Scene>& scene);
		void OnImGuiRender();

		void DrawSceneHierarchy();

		Entity GetSelectedEntity() { return m_SelectionContext; }
		void SetSelectedEntity(Entity entity) { m_SelectionContext = entity; }

		bool GetShowGlobalTransform() { return m_ShowGlobal; }
		void SetShowGlobalTransform(bool show) { m_ShowGlobal = show; }

		bool IsFocused() { return m_HierarchyFocused; }
		bool IsHovered() { return m_HierarchyHovered; }
	private:
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;

		bool m_ShowGlobal = false;
		bool m_HierarchyFocused = false;
		bool m_HierarchyHovered = false;

		int32_t m_MovedEntityIndex = -1;
		
		friend class Scene;
	};
}