#pragma once

#include <Nebula.h>

namespace Nebula {
	struct RectData;

	class SceneHierarchyPanel {
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void SetContext(const Ref<Scene>& scene);
		void OnImGuiRender();

		void DrawSceneHierarchy();

		Entity GetSelectedEntity() { return m_SelectionContext; }
		void SetSelectedEntity(Entity entity) { m_SelectionContext = entity; }

		void DisplayCreateEntity(Entity parent = {});

		bool IsFocused() { return m_HierarchyFocused; }
		bool IsHovered() { return m_HierarchyHovered; }
	private:
		template<typename T>
		void DisplayAddComponentEntry(const std::string& entryName);

		void DrawEntityNode(Entity entity, uint32_t index);
		void DrawArray(Array<UUID>& entities);
		void DrawComponents(Entity entity);
	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;

		Array<RectData*> Rects;

		bool m_HierarchyFocused = false;
		bool m_HierarchyHovered = false;

		int32_t m_MovedEntityIndex = -1;
		
		friend class Scene;
	};
}