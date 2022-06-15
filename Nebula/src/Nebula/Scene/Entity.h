#pragma once

#include "Scene.h"
#include "Components.h"

namespace Nebula {
	class Entity {
	public:
		Entity() = default;
		Entity(const Entity & other) = default;

		Entity(entt::entity handle, Scene* scene) : m_EntityHandle(handle), m_Scene(scene) { }
		Entity(UUID uuid, Scene* scene) : m_Scene(scene) {
			auto& view = m_Scene->m_Registry.view<IDComponent>();
			for (auto& ent : view) {
				Entity entity{ ent, m_Scene };
				if (entity.GetUUID() == uuid) {
					m_EntityHandle = entity;
					break;
				}
			}
		}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args) {
			NB_ASSERT(!HasComponent<T>(), "Entity already has component!");
		
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			
			return component;
		}

		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args) {
			T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);

			return component;
		}

		template<typename T>
		T& GetComponent() {
			NB_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent() {
			return m_Scene->m_Registry.has<T>(m_EntityHandle);
		}

		template<typename T>
		void RemoveComponent() {
			NB_ASSERT(HasComponent<T>(), "Entity does not have component!");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}
		
		const std::string& GetName() { return GetComponent<TagComponent>().Tag; }
		UUID GetUUID() { return GetComponent<IDComponent>().ID; }
		TransformComponent& GetTransform() { return GetComponent<TransformComponent>(); }
		ParentChildComponent& GetParentChild() { return GetComponent<ParentChildComponent>(); }

		operator bool() const { return m_EntityHandle != entt::null; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }
		operator int() const { return (int)m_EntityHandle; }
		operator entt::entity() const { return m_EntityHandle; }
		operator Scene*() const { return m_Scene; }

		bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }
		bool operator!=(const Entity& other) const { return !(*this == other); }
	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;
	};

	void CalculateGlobalTransform(Entity& entity);
	void UpdateChildrenAndTransform(Entity& entity);

	class ScriptableEntity {
	public:
		virtual ~ScriptableEntity() { }

		template<typename T>
		T& GetComponent() {
			return m_Entity.GetComponent<T>();
		}

		void DestroyObject(ScriptableEntity* obj) {
			m_Scene->DestroyEntity(obj->m_Entity);
		}

		void DestroyObject(Entity entity) {
			m_Scene->DestroyEntity(entity);
		}

		Scene* GetActiveScene() { return m_Scene; }
		Entity GetCurrentEntity() { return m_Entity; }
	protected:
		virtual void Start() { }
		virtual void Update() { }
		virtual void Destroy() { }
		virtual void OnCollisionEnter(Entity other) { }
		virtual void OnCollisionExit(Entity other) { }
	private:
		Entity m_Entity;
		Scene* m_Scene;
		friend class Scene;
		friend class ContactListener;
	};
}