#pragma once

#include "entt.hpp"
#include "Nebula/Renderer/Camera.h"

class b2World;

namespace Nebula {
	class Entity;
	class UUID;
	
	class Scene {
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name);
		Entity CreateEntity(UUID uuid, const std::string& name);
		void DestroyEntity(Entity entity);
		
		void OnRuntimeStart();
		void OnRuntimeStop();

		void UpdateRuntime();
		void UpdateEditor(EditorCamera& camera);
		void RenderRuntime();
		
		void OnViewportResize(uint32_t width, uint32_t height);

		Entity GetPrimaryCamera();
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