#include "nbpch.h"
#include "Scene.h"

#include "Components.h"
#include "Entity.h"

#include "Nebula/Renderer/Renderer2D.h"

#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"

#include "Nebula/Utils/Time.h"

namespace Nebula {
	void CalculateGlobalTransform(Entity& entity) {
		auto& transform = entity.GetTransform();
		UUID parentID = entity.GetParentChild().Parent;

		transform.global = transform.CalculateLocalMatrix();
		if (parentID) {
			Entity parent = { parentID, entity };
			CalculateGlobalTransform(parent);
			auto& p = parent.GetTransform();
			transform.global = p.global * transform.CalculateLocalMatrix();
		}

		DecomposeTransform(transform.global, transform.GlobalTranslation, transform.GlobalRotation, transform.GlobalScale);
	}

	void UpdateChildTransform(Entity& entity) {
		if (entity.HasComponent<Rigidbody2DComponent>()) {
			Rigidbody2DComponent& rb2d = entity.GetComponent<Rigidbody2DComponent>();
			if (rb2d.RuntimeBody) {
				TransformComponent& transform = entity.GetComponent<TransformComponent>();

				b2Body* body = (b2Body*)rb2d.RuntimeBody;
				body->SetTransform({ transform.GlobalTranslation.x, transform.GlobalTranslation.y }, transform.GlobalRotation.z);
			}
		}

		for (UUID child : entity.GetParentChild().ChildrenIDs) {
			Entity c = { child, entity };
			if (!c.GetParentChild().ChildrenIDs.size())
				CalculateGlobalTransform(c);
			else
				UpdateChildTransform(c);
		}
	}

	void Rigidbody2DComponent::ApplyAngularImpulse(float impulse) {
		((b2Body*)RuntimeBody)->ApplyAngularImpulse(impulse, true);
	}

	void Rigidbody2DComponent::ApplyForce(vec2 force, vec2 point) {
		((b2Body*)RuntimeBody)->ApplyForce({ force.x, force.y }, { point.x, point.y }, true);
	}

	void Rigidbody2DComponent::ApplyLinearImpulse(vec2 impulse, vec2 point) {
		((b2Body*)RuntimeBody)->ApplyLinearImpulse({ impulse.x, impulse.y }, { point.x, point.y }, true);
	}

	void Rigidbody2DComponent::ApplyTorque(float torque) {
		((b2Body*)RuntimeBody)->ApplyTorque(torque, true);
	}

	void Rigidbody2DComponent::ApplyForceToCenter(vec2 force) {
		((b2Body*)RuntimeBody)->ApplyForceToCenter({ force.x, force.y }, true);
	}

	void Rigidbody2DComponent::ApplyLinearImpulseToCenter(vec2 impulse) {
		((b2Body*)RuntimeBody)->ApplyLinearImpulseToCenter({ impulse.x, impulse.y }, true);
	}

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

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& map) {
		([&]()
			{
				auto view = src.view<Component>();
				for (auto srcEntity : view)
				{
					entt::entity dstEntity = map.at(src.get<IDComponent>(srcEntity).ID);

					auto& srcComponent = src.get<Component>(srcEntity);
					dst.emplace_or_replace<Component>(dstEntity, srcComponent);
				}
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap) {
		CopyComponent<Component...>(dst, src, enttMap);
	}

	template<typename... Component>
	static void CopyComponent(Entity dst, Entity src) {
		([&]()
		{
			if (src.HasComponent<Component>())
				dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
		}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, Entity dst, Entity src)
	{
		CopyComponent<Component...>(dst, src);
	}

	Ref<Scene> Scene::Copy(Ref<Scene> other) {
		Ref<Scene> newScene = CreateRef<Scene>();
		newScene->m_ViewportWidth = other->m_ViewportWidth;
		newScene->m_ViewportHeight = other->m_ViewportHeight;

		std::unordered_map<UUID, entt::entity> enttMap;

		auto& srcSceneReg = other->m_Registry;
		auto& dstSceneReg = newScene->m_Registry;
		auto idView = srcSceneReg.view<IDComponent>();

		Array<entt::entity> entityVec;

		for (auto e : idView)
			entityVec.push_back(e);

		for (size_t i = entityVec.size(); i; i--) {
			UUID uuid = srcSceneReg.get<IDComponent>(entityVec[i - 1]).ID;
			const auto& name = srcSceneReg.get<TagComponent>(entityVec[i -1]).Tag;
			Entity newEnt = newScene->CreateEntity(uuid, name);
			enttMap[uuid] = (entt::entity)newEnt;
		}

		CopyComponent(AllComponents{}, dstSceneReg, srcSceneReg, enttMap);
		newScene->m_SceneOrder = other->m_SceneOrder;

		return newScene;
	}

	Scene::Scene() { }

	Scene::~Scene() { }

	Entity Scene::CreateEntity(const std::string& name) {
		return CreateEntity(UUID(), name);
	}

	Entity Scene::CreateEntity(UUID uuid, const std::string& name) {
		Entity entity = { m_Registry.create(), this };
		m_SceneOrder.push_back(uuid);

		auto& idc = entity.AddComponent<IDComponent>();
		idc.ID = uuid;
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<ParentChildComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity) {
		auto& Parent = entity.GetComponent<ParentChildComponent>();

		for (auto& childID : Parent.ChildrenIDs)
			DestroyEntity({ childID, this });

		if (Parent.Parent) {
			Entity{ Parent.Parent, this }.GetComponent<ParentChildComponent>().RemoveChild(entity.GetUUID());
			Parent.Parent = NULL;
		}

		Array<UUID> newVec;
		for (uint32_t i = 0; i < m_SceneOrder.size(); i++) {
			if (m_SceneOrder[i] != entity.GetUUID())
				newVec.push_back(m_SceneOrder[i]);
		}

		m_SceneOrder = newVec;
		m_Registry.destroy(entity);
	}

	Entity Scene::DuplicateEntity(Entity entity) {
		std::string name = entity.GetName();
		Entity newEnt = CreateEntity(name);

		CopyComponent(AllComponents{}, newEnt, entity);
		
		ParentChildComponent& pcc = newEnt.GetParentChild();
		pcc.ChildrenIDs.clear();
		pcc.Parent = NULL;

		for (UUID& childID : entity.GetParentChild().ChildrenIDs) {
			Entity child = { childID, this };

			Entity newChild = DuplicateEntity(child);
			
			newEnt.GetParentChild().AddChild(newChild.GetUUID());
			newChild.GetParentChild().Parent = newEnt.GetUUID();
		}

		return newEnt;
	}

	void Scene::OnRuntimeStart() {
		m_PhysicsWorld = new b2World({ 0.0f, -9.81f });
		m_PhysicsWorld->SetAllowSleeping(false);

		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view) {
			Entity entity = { e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			b2BodyDef bodyDef;
			bodyDef.type = Rigibody2DToBox2D(rb2d.Type);
			bodyDef.position.Set(transform.GlobalTranslation.x, transform.GlobalTranslation.y);
			bodyDef.angle = transform.GlobalRotation.z;

			b2Body* body;
			if (entity.HasComponent<Box2DComponent>()) {
				auto& bc2d = entity.GetComponent<Box2DComponent>();

				bodyDef.position.x += bc2d.Offset.x;
				bodyDef.position.y += bc2d.Offset.y;
				body = m_PhysicsWorld->CreateBody(&bodyDef);

				b2PolygonShape polygonShape;
				polygonShape.SetAsBox(transform.GlobalScale.x * bc2d.Size.x, transform.GlobalScale.y * bc2d.Size.y); 
				
				b2FixtureDef fixtureDef;
				fixtureDef.shape = &polygonShape;
				fixtureDef.density = rb2d.Density;
				fixtureDef.friction = rb2d.Friction;
				fixtureDef.restitution = rb2d.Restitution;
				fixtureDef.restitutionThreshold = rb2d.RestitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}

			if (entity.HasComponent<CircleColliderComponent>()) {
				auto& cc = entity.GetComponent<CircleColliderComponent>();

				bodyDef.position.x += cc.Offset.x;
				bodyDef.position.y += cc.Offset.y;
				body = m_PhysicsWorld->CreateBody(&bodyDef);

				b2CircleShape circle;
				circle.m_p.Set(cc.Offset.x, cc.Offset.y);
				circle.m_radius = cc.Radius * entity.GetComponent<TransformComponent>().GlobalScale.x;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &circle;
				fixtureDef.density = rb2d.Density;
				fixtureDef.friction = rb2d.Friction;
				fixtureDef.restitution = rb2d.Restitution;
				fixtureDef.restitutionThreshold = rb2d.RestitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}

			body->SetFixedRotation(rb2d.FixedRotation);
			rb2d.RuntimeBody = body;
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

	void Scene::UpdatePhysics() {
		m_PhysicsWorld->Step(Time::DeltaTime(), 6, 2);

		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view) {
			Entity entity = { e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			auto position = body->GetPosition();

			if (entity.HasComponent<Box2DComponent>()) {
				auto& bc2d = entity.GetComponent<Box2DComponent>();

				position.x -= bc2d.Offset.x;
				position.y -= bc2d.Offset.y;
			}

			if (entity.HasComponent<CircleColliderComponent>()) {
				auto& cc = entity.GetComponent<CircleColliderComponent>();

				position.x -= cc.Offset.x;
				position.y -= cc.Offset.y;
			}

			vec3 deltaTranslation = { position.x - transform.GlobalTranslation.x ,
				position.y - transform.GlobalTranslation.y, 0.0f };
			vec3 deltaRotation = { 0.0f, 0.0f, body->GetAngle() - transform.GlobalRotation.z };

			transform.SetDeltaTransform(deltaTranslation, deltaRotation, vec3(0.0f));
		}
	}

	void Scene::UpdateRuntime() {
		auto camView = m_Registry.view<TransformComponent, CameraComponent>();
		for (auto entity : camView) {
			auto [transform, camera] = camView.get<TransformComponent, CameraComponent>(entity);

			if (camera.Primary) {
				mainCam = &camera.Camera;
				mainCamTransform = transform;
				break;
			}
		}

		m_Registry.view<NativeScriptComponent>().each([=](auto entity, NativeScriptComponent& nsc) {
			if (!nsc.Instance) {
				nsc.Instance = nsc.InstantiateScript();
				nsc.Instance->m_Entity = Entity{ entity, this };
				nsc.Instance->Start();
			}

			nsc.Instance->Update();
		});

		UpdatePhysics();
	}

	void Scene::RenderEditor(EditorCamera& camera) {
		Renderer2D::BeginScene(camera);
		
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

	void Scene::UpdateEditor() { }

	void Scene::RenderRuntime() {
		if (!mainCam) return;

		Renderer2D::BeginScene(*mainCam, mainCamTransform);

		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group) {
			Renderer2D::Draw(NB_QUAD, Entity{ entity, this });
		}

		auto CircleGroup = m_Registry.view<CircleRendererComponent>();
		for (auto entity : CircleGroup) {
			Renderer2D::Draw(NB_CIRCLE, Entity{ entity, this });
		}

		Renderer2D::EndScene();
	}

	void Scene::RenderEditorOverlay(EditorCamera& camera) {
		Renderer2D::BeginScene(camera);
		auto StringGroup = m_Registry.view<StringRendererComponent>();
		for (auto entity : StringGroup) {
			Renderer2D::Draw(NB_STRING, Entity{ entity, this });
		}
		Renderer2D::EndScene();
	}

	void Scene::RenderRuntimeOverlay() {
		if (!mainCam) return;
		
		Renderer2D::BeginScene(*mainCam, mainCamTransform);
		auto StringGroup = m_Registry.view<StringRendererComponent>();
		for (auto entity : StringGroup) {
			Renderer2D::Draw(NB_STRING, Entity{ entity, this });
		}
		Renderer2D::EndScene();
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
	void Scene::OnComponentAdded<StringRendererComponent>(Entity entity, StringRendererComponent& component) { }

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
	void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component) { }

	template<>
	void Scene::OnComponentAdded<Box2DComponent>(Entity entity, Box2DComponent& component) { }

	template<>
	void Scene::OnComponentAdded<CircleColliderComponent>(Entity entity, CircleColliderComponent& component) { }

	template<>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component) { }

	template<>
	void Scene::OnComponentAdded<ParentChildComponent>(Entity entity, ParentChildComponent& component) { }
}