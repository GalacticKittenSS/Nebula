#pragma once

#include "entt.hpp"

#include "Components.h"

#include "Nebula/Renderer/Camera.h"
#include "Nebula/Core/UUID.h"

#include <map>

class b2World;
class b2Body;

namespace Nebula {
	class Entity;
	class ContactListener;
	
	class Scene {
	public:
		struct SceneNode
		{
			UUID ID;
			UUID Parent = NULL;
			Array<UUID> Children;

			SceneNode() = default;
			SceneNode(const SceneNode&) = default;
			SceneNode(UUID id)
				: ID(id) {}
		};
	public:
		Scene();
		~Scene();

		static Ref<Scene> Copy(Ref<Scene> other);

		Entity CreateEntity(std::string_view name);
		Entity CreateEntity(UUID uuid, std::string_view name);
		void DestroyEntity(Entity entity);

		Entity DuplicateEntity(Entity entity, bool duplicateIntoParent = true);
		
		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

		void UpdateSimulation();
		void UpdateEditor();
		void UpdateRuntime();

		void OnViewportResize(uint32_t width, uint32_t height);

		Entity GetPrimaryCamera();
		Entity GetEntityWithTag(std::string_view tag);
		Entity GetEntityWithUUID(UUID id);

		template <typename... Components>
		auto GetAllEntitiesWith() {
			return m_Registry.view<Components...>();
		}

		SceneNode& GetEntityNode(UUID entityID);
		
		void SetPaused(bool paused) { m_IsPaused = paused; }
		bool IsPaused() const { return m_IsPaused; }
		bool IsRunning() const { return m_IsRunning; }

		void Step(int frames = 1);

		Array<UUID> m_SceneOrder;
	private:
		void CreateBox2DBody(Entity entity);
		void UpdateBox2DBody(Entity entity);

		void InitPhysics();
		void UpdatePhysics();
		void DestroyPhysics();

		void InitScripts();
		void UpdateScripts();
		void DestroyScripts();

		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	private:
		entt::registry m_Registry;
		std::unordered_map<UUID, entt::entity> m_EntityMap;
		std::unordered_map<UUID, SceneNode> m_Nodes;

		bool m_IsRunning = false, m_IsPaused = false;
		int m_StepFrames = 0;

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		b2World* m_PhysicsWorld = nullptr;
		ContactListener* m_ContactListener = nullptr;

		Camera* mainCam = nullptr;
		glm::mat4 mainCamTransform;

		friend class Entity;
		friend class SceneRenderer;
		friend class SceneHierarchyPanel;
		friend class SceneSerializer;
		friend class PrefabSerializer;
		friend class ScriptEngine;
	};
}