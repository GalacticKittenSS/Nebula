#include "nbpch.h"
#include "Scene.h"

#include "Nebula/Renderer/Renderer2D.h"
#include "Nebula/Utils/Time.h"

#include "Components.h"
#include "Entity.h"
#include "ContactListener.h"

#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"

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

	Scene::Scene() { }

	Scene::~Scene() {
		DestroyPhysics();
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

		if (entity.HasComponent<Rigidbody2DComponent>() && entity.GetComponent<Rigidbody2DComponent>().RuntimeBody) {
			entity.GetComponent<Rigidbody2DComponent>().RuntimeBody = nullptr;
			CreateBox2DBody(entity);
		}

		return newEnt;
	}

	void Scene::DestroyEntity(Entity entity) {
		if (entity.HasComponent<NativeScriptComponent>() && entity.GetComponent<NativeScriptComponent>().Instance)
			entity.GetComponent<NativeScriptComponent>().DestroyScript(&entity.GetComponent<NativeScriptComponent>());

		if (entity.HasComponent<Rigidbody2DComponent>() && entity.GetComponent<Rigidbody2DComponent>().RuntimeBody) {
			delete (Entity*)((b2Body*)entity.GetComponent<Rigidbody2DComponent>().RuntimeBody)->GetUserData().pointer;
			((b2Body*)entity.GetComponent<Rigidbody2DComponent>().RuntimeBody)->GetUserData().pointer = reinterpret_cast<uintptr_t>(nullptr);
			m_PhysicsWorld->DestroyBody((b2Body*)entity.GetComponent<Rigidbody2DComponent>().RuntimeBody);
		}
		
		auto& Parent = entity.GetComponent<ParentChildComponent>();
		for (auto& childID : Parent.ChildrenIDs)
			DestroyEntity({ childID, this });

		if (Parent.Parent) {
			Entity{ Parent.Parent, this }.GetComponent<ParentChildComponent>().RemoveChild(entity.GetUUID());
			Parent.Parent = NULL;
		}

		m_SceneOrder.remove(entity.GetUUID());
		m_Registry.destroy(entity);
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

	Entity Scene::GetEntityWithTag(std::string tag) {
		for (auto& entityID : GetAllEntitiesWith<TagComponent>()) {
			Entity entity{ entityID, this };

			if (entity.GetName() == tag)
				return entity;
		}

		NB_ERROR("Could Not Find Entity with name {0}", tag);
		return Entity{};
	}

	void Scene::CreateBox2DBody(Entity entity) {
		auto& transform = entity.GetComponent<TransformComponent>();
		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

		b2BodyDef bodyDef;
		bodyDef.type = Rigibody2DToBox2D(rb2d.Type);
		bodyDef.position.Set(transform.GlobalTranslation.x, transform.GlobalTranslation.y);
		bodyDef.angle = transform.GlobalRotation.z;

		b2BodyUserData data;
		data.pointer = reinterpret_cast<uintptr_t>(new Entity(entity, this));
		bodyDef.userData = data;

		b2Body* body;
		if (entity.HasComponent<BoxCollider2DComponent>()) {
			auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

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

			fixtureDef.filter.categoryBits = bc2d.Category;
			fixtureDef.filter.maskBits = bc2d.Mask;
			
			bc2d.RuntimeFixture = body->CreateFixture(&fixtureDef);
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

			fixtureDef.filter.categoryBits = cc.Category;
			fixtureDef.filter.maskBits = cc.Mask;

			cc.RuntimeFixture = body->CreateFixture(&fixtureDef);
		}

		body->SetFixedRotation(rb2d.FixedRotation);
		rb2d.RuntimeBody = body;
	}

	void Scene::InitPhysics() {
		m_PhysicsWorld = new b2World({ 0.0f, -9.81f });
		m_PhysicsWorld->SetAllowSleeping(false);
		m_PhysicsWorld->SetContactListener(new ContactListener());

		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view)
			CreateBox2DBody(Entity{ e, this });
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

			if (entity.HasComponent<BoxCollider2DComponent>()) {
				auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

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

	void Scene::DestroyPhysics() {
		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view) {
			Entity entity = { e, this };
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
			if (rb2d.RuntimeBody) {
				delete (Entity*)((b2Body*)rb2d.RuntimeBody)->GetUserData().pointer;
				rb2d.RuntimeBody = nullptr;
			}
		}

		delete m_PhysicsWorld; m_PhysicsWorld = nullptr;
	}

	void Scene::OnRuntimeStart() {
		InitPhysics();

		m_Registry.view<NativeScriptComponent>().each([=](auto entity, NativeScriptComponent& nsc) {
			nsc.Instance = nsc.InstantiateScript();
			nsc.Instance->m_Entity = Entity{ entity, this };
			nsc.Instance->m_Scene = this;
			nsc.Instance->Start();
		});
	}

	void Scene::OnRuntimeStop() {
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, NativeScriptComponent& nsc) {
			if (!nsc.Instance)
				return;

			nsc.Instance->Destroy();
			nsc.DestroyScript(&nsc);
		});

		DestroyPhysics();
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
				nsc.Instance->m_Scene = this;
				nsc.Instance->Start();
			}

			nsc.Instance->Update();
		});

		UpdatePhysics();
	}

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

	void Scene::RenderRuntimeOverlay() {
		if (!mainCam) return;

		Renderer2D::BeginScene(*mainCam, mainCamTransform);
		auto StringGroup = m_Registry.view<StringRendererComponent>();
		for (auto entity : StringGroup) {
			Renderer2D::Draw(NB_STRING, Entity{ entity, this });
		}
		Renderer2D::EndScene();
	}

	void Scene::OnSimulationStart() {
		InitPhysics();
	}

	void Scene::OnSimulationStop() {
		DestroyPhysics();
	}

	void Scene::UpdateSimulation() {
		UpdatePhysics();
	}

	void Scene::UpdateEditor() { }

	void Scene::Render(EditorCamera& camera) {
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

	void Scene::RenderOverlay(EditorCamera& camera) {
		Renderer2D::BeginScene(camera);

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
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component) { }

	template<>
	void Scene::OnComponentAdded<CircleColliderComponent>(Entity entity, CircleColliderComponent& component) { }

	template<>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component) { }

	template<>
	void Scene::OnComponentAdded<ParentChildComponent>(Entity entity, ParentChildComponent& component) { }
}