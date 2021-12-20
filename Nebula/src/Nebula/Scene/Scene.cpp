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
	Scene::Scene() {

	}

	Scene::~Scene() {

	}

	Entity Scene::CreateEntity(const std::string& name) {
		Entity entity =  { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity": name;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity) {
		m_Registry.destroy(entity);
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
			auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

			Renderer2D::Draw(NB_QUAD, Entity{ entity, this });
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
				auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::Draw(NB_QUAD, Entity{ entity, this });
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
}