#include "nbpch.h"
#include "Scene.h"

#include "Components.h"
#include "Entity.h"

#include "Nebula/Renderer/Renderer2D.h"

#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"

#include "Nebula/Core/Time.h"

namespace Nebula {
	static b2BodyType Rigibody2DToBox2D(Rigidbody2DComponent::BodyType bodyType) {
		switch (bodyType)
		{
			case Rigidbody2DComponent::BodyType::Static:	return b2BodyType::b2_staticBody;
			case Rigidbody2DComponent::BodyType::Dynamic:	return b2BodyType::b2_dynamicBody;
			case Rigidbody2DComponent::BodyType::Kinematic: return b2BodyType::b2_kinematicBody;
		}
		
		NB_ASSERT(false, "Unknown Rigidbody Type!");
		return b2BodyType::b2_staticBody;
	}

	template<typename T>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& map) {
		auto view = src.view<T>();

		for (auto e : view) {
			UUID uuid = src.get<IDComponent>(e).ID;
			
			NB_ASSERT(map.find(uuid) != map.end(), "");
			entt::entity dstEnttID = map.at(uuid);

			auto& Component = src.get<T>(e);
			dst.emplace_or_replace<T>(dstEnttID, Component);

		}
	}

	template<typename T>
	static void CopyComponent(Entity dst, Entity src) {
		if (src.HasComponent<T>())
			dst.AddOrReplaceComponent<T>(src.GetComponent<T>());
	}

	Ref<Scene> Scene::Copy(Ref<Scene> other) {
		Ref<Scene> newScene = CreateRef<Scene>();
		newScene->m_ViewportWidth = other->m_ViewportWidth;
		newScene->m_ViewportHeight = other->m_ViewportHeight;

		std::unordered_map<UUID, entt::entity> enttMap;

		auto& srcSceneReg = other->m_Registry;
		auto& dstSceneReg = newScene->m_Registry;
		auto idView = srcSceneReg.view<IDComponent>();
		for (auto e : idView) {
			UUID uuid = srcSceneReg.get<IDComponent>(e).ID;
			const auto& name = srcSceneReg.get<TagComponent>(e).Tag;
			Entity newEnt = newScene->CreateEntity(uuid, name);
			enttMap[uuid] = (entt::entity)newEnt;
		}

		CopyComponent<TransformComponent>(dstSceneReg, srcSceneReg, enttMap);
		CopyComponent<SpriteRendererComponent>(dstSceneReg, srcSceneReg, enttMap);
		CopyComponent<CircleRendererComponent>(dstSceneReg, srcSceneReg, enttMap);
		CopyComponent<CameraComponent>(dstSceneReg, srcSceneReg, enttMap);
		CopyComponent<NativeScriptComponent>(dstSceneReg, srcSceneReg, enttMap);
		CopyComponent<Rigidbody2DComponent>(dstSceneReg, srcSceneReg, enttMap);
		CopyComponent<Box2DComponent>(dstSceneReg, srcSceneReg, enttMap);

		return newScene;
	}

	Scene::Scene() {

	}

	Scene::~Scene() {

	}

	Entity Scene::CreateEntity(const std::string& name) {
		return CreateEntity(UUID(), name);
	}

	Entity Scene::CreateEntity(UUID uuid, const std::string& name) {
		Entity entity = { m_Registry.create(), this };
		auto& idc = entity.AddComponent<IDComponent>();
		idc.ID = uuid;
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity) {
		m_Registry.destroy(entity);
	}

	void Scene::DuplicateEntity(Entity entity) {
		std::string name = entity.GetName();
		Entity newEnt = CreateEntity(name);

		CopyComponent<TransformComponent>(newEnt, entity);
		CopyComponent<SpriteRendererComponent>(newEnt, entity);
		CopyComponent<CircleRendererComponent>(newEnt, entity);
		CopyComponent<CameraComponent>(newEnt, entity);
		CopyComponent<NativeScriptComponent>(newEnt, entity);
		CopyComponent<Rigidbody2DComponent>(newEnt, entity);
		CopyComponent<Box2DComponent>(newEnt, entity);
	}

	void Scene::OnRuntimeStart() {
		m_PhysicsWorld = new b2World({ 0.0f, -9.8f });
		
		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view) {
			Entity entity = { e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			b2BodyDef bodyDef;
			bodyDef.type = Rigibody2DToBox2D(rb2d.Type);
			bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
			bodyDef.angle = transform.Rotation.z;

			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
			body->SetFixedRotation(rb2d.FixedRotation);
			rb2d.RuntimeBody = body;

			if (entity.HasComponent<Box2DComponent>()) {
				auto& bc2d = entity.GetComponent<Box2DComponent>();

				b2PolygonShape polygonShape;
				polygonShape.SetAsBox(transform.Scale.x * bc2d.Size.x, transform.Scale.y * bc2d.Size.y);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &polygonShape;
				fixtureDef.density = bc2d.Density;
				fixtureDef.friction = bc2d.Friction;
				fixtureDef.restitution = bc2d.Restitution;
				fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}
		}
	}

	void Scene::OnRuntimeStop() {
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}

	Entity Scene::GetPrimaryCamera() {
		auto view = m_Registry.view<CameraComponent>();

		for (auto entity : view) {
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary) {
				return Entity{ entity, this };
			}
		}

		return {};
	}

	void Scene::UpdateRuntime() {
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, NativeScriptComponent& nsc)
		{
			if (!nsc.Instance) {
				nsc.Instance = nsc.InstantiateScript();
				nsc.Instance->m_Entity = Entity{ entity, this };
				nsc.Instance->Start();
			}

			nsc.Instance->Update();
		});

		const int32_t velocityIterations = 6;
		const int32_t positionIterations = 2;
		m_PhysicsWorld->Step(Time::DeltaTime(), velocityIterations, positionIterations);

		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view) {
			Entity entity = { e, this };
			auto& tranform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			const auto& position = body->GetPosition();
			tranform.Translation.x = position.x;
			tranform.Translation.y = position.y;
			tranform.Rotation.z = body->GetAngle();
		}
	}

	void Scene::UpdateEditor(EditorCamera& camera) {
		Renderer2D::BeginScene(camera); 

		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group) {
			Renderer2D::Draw(NB_RECT, Entity{ entity, this });
		}

		auto CircleGroup = m_Registry.view<TransformComponent, CircleRendererComponent>();
		for (auto entity : CircleGroup) {
			Renderer2D::Draw(NB_RECT, Entity{ entity, this });
		}

		Renderer2D::EndScene();
	}

	void Scene::RenderRuntime() {
		Camera* mainCam = nullptr;
		mat4 mainCamTransform;

		auto view = m_Registry.view<TransformComponent, CameraComponent>();
		for (auto entity : view) {
			auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

			if (camera.Primary) {
				mainCam = &camera.Camera;
				mainCamTransform = transform;
				break;
			}
		}

		if (mainCam) {
			Renderer2D::BeginScene(*mainCam, mainCamTransform);

			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group) {
				Renderer2D::Draw(NB_QUAD, Entity{ entity, this });
			}

			auto CircleGroup = m_Registry.view<TransformComponent, CircleRendererComponent>();
			for (auto entity : CircleGroup) {
				Renderer2D::Draw(NB_CIRCLE, Entity{ entity, this });
			}

			Renderer2D::EndScene();
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height) {
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view) {
			auto& cameraComponent = view.get<CameraComponent>(entity);
			
			if (!cameraComponent.FixedAspectRatio) {
				cameraComponent.Camera.SetViewPortSize(width, height);
			}
		}
	}


	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component) {
		static_assert(false);
	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component) { }

	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component) { }

	template<>
	void Scene::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent& component) { }

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component) { }

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component) { }

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component) { 
		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			component.Camera.SetViewPortSize(m_ViewportWidth, m_ViewportHeight);
	}

	template<>
	void Scene::OnComponentAdded<Box2DComponent>(Entity entity, Box2DComponent& component) { }

	template<>
	void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component) { }
	
	template<>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component) { }
}