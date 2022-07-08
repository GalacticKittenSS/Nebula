#pragma once

#include "entt.hpp"

#include "Nebula/Renderer/Camera.h"
#include "Nebula/Core/UUID.h"

class b2World;

namespace Nebula {
	class Entity;
	
	class Scene {
	public:
		Scene();
		~Scene();

		static Ref<Scene> Copy(Ref<Scene> other);

		Entity CreateEntity(const std::string& name);
		Entity CreateEntity(UUID uuid, const std::string& name);
		void DestroyEntity(Entity entity);

		Entity DuplicateEntity(Entity entity);
		
		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

		void UpdateSimulation();
		void UpdateEditor();
		void UpdateRuntime();

		void RenderRuntime();
		void RenderRuntimeOverlay();

		void Render(EditorCamera& camera);
		void RenderOverlay(EditorCamera& camera);

		void Render(const Camera& camera, const mat4& transform);
		void RenderOverlay(const Camera& camera, const mat4& transform);
		
		void OnViewportResize(uint32_t width, uint32_t height);

		Entity GetPrimaryCamera();
		Entity GetEntityWithTag(std::string tag);

		template <typename... Components>
		auto GetAllEntitiesWith() {
			return m_Registry.view<Components...>();
		}
		
		Array<UUID> m_SceneOrder;
	private:
		void CreateBox2DBody(Entity entity);

		void InitPhysics();
		void UpdatePhysics();
		void DestroyPhysics();

		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		b2World* m_PhysicsWorld = nullptr;

		Camera* mainCam = nullptr;
		mat4 mainCamTransform;

		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class SceneSerializer;
	};
}