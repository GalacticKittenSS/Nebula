#pragma once

#include "entt.hpp"
#include "Nebula/Renderer/Camera.h"
#include "Nebula/Utils/Arrays.h"

class b2World;

namespace Nebula {
	class Entity;
	class UUID;
	
	class Scene {
	public:
		Scene();
		~Scene();

		static Ref<Scene> Copy(Ref<Scene> other);

		Entity CreateEntity(const std::string& name);
		Entity CreateEntity(UUID uuid, const std::string& name);
		void DestroyEntity(Entity entity);

		void DuplicateEntity(Entity entity);
		
		void OnRuntimeStart();
		void OnRuntimeStop();

		void UpdateRuntime();
		void UpdateEditor();
		void RenderEditor(EditorCamera& camera);
		void RenderRuntime();
		
		void OnViewportResize(uint32_t width, uint32_t height);

		Entity GetPrimaryCamera();

		template <typename... Components>
		auto GetAllEntitiesWith() {
			return m_Registry.view<Components...>();
		}

		Array<UUID> m_SceneOrder;
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		b2World* m_PhysicsWorld = nullptr;

		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class SceneSerializer;
	};
}