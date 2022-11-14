#include "nbpch.h"
#include "Scene.h"

#include "Nebula/Renderer/Renderer2D.h"
#include "Nebula/Utils/Time.h"
#include "Nebula/Scripting/ScriptEngine.h"

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

	Entity Scene::CreateEntity(std::string_view name) {
		return CreateEntity(UUID(), name);
	}

	Entity Scene::CreateEntity(UUID uuid, std::string_view name) {
		Entity entity = { m_Registry.create(), this };
		m_SceneOrder.push_back(uuid); 
		m_EntityMap[uuid] = entity;

		auto& idc = entity.AddComponent<IDComponent>();
		idc.ID = uuid;
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<WorldTransformComponent>();
		entity.AddComponent<ParentChildComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	Entity Scene::DuplicateEntity(Entity entity, bool duplicateIntoParent) {
		std::string name = entity.GetName();
		Entity duplicated = CreateEntity(name);

		CopyComponent(AllComponents{}, duplicated, entity);

		const UUID& duplicatedID = duplicated.GetUUID();

		ParentChildComponent& pcc = entity.GetParentChild();
		if (pcc.Parent && duplicateIntoParent)
		{
			duplicated.GetParentChild().Parent = pcc.Parent;
			
			Entity parent = { pcc.Parent, this };
			parent.GetParentChild().AddChild(duplicatedID);

			m_SceneOrder.remove(duplicatedID);
		}
		
		for (uint32_t i = 0; i < entity.GetParentChild().ChildrenIDs.size(); i++)
		{
			Entity child = DuplicateEntity({ entity.GetParentChild().ChildrenIDs[i], this }, false);
			UUID childID = child.GetUUID();

			duplicated.GetParentChild().AddChild(childID);
			child.GetParentChild().Parent = duplicatedID;
			m_SceneOrder.remove(childID);
		}
		
		if (entity.HasComponent<Rigidbody2DComponent>() && entity.GetComponent<Rigidbody2DComponent>().RuntimeBody) {
			entity.GetComponent<Rigidbody2DComponent>().RuntimeBody = nullptr;
			CreateBox2DBody(entity);
		}

		return duplicated;
	}

	void Scene::DestroyEntity(Entity entity) {
		if (entity.HasComponent<NativeScriptComponent>() && entity.GetComponent<NativeScriptComponent>().Instance)
			entity.GetComponent<NativeScriptComponent>().DestroyScript(&entity.GetComponent<NativeScriptComponent>());

		if (entity.HasComponent<Rigidbody2DComponent>() && entity.GetComponent<Rigidbody2DComponent>().RuntimeBody) {
			delete (Entity*)((b2Body*)entity.GetComponent<Rigidbody2DComponent>().RuntimeBody)->GetUserData().pointer;
			((b2Body*)entity.GetComponent<Rigidbody2DComponent>().RuntimeBody)->GetUserData().pointer = reinterpret_cast<uintptr_t>(nullptr);
			m_PhysicsWorld->DestroyBody((b2Body*)entity.GetComponent<Rigidbody2DComponent>().RuntimeBody);
		}
		
		auto& Parent = entity.GetParentChild();
		size_t size = Parent.ChildrenIDs.size();
		for (size_t i = 0; i < size; i++)
			DestroyEntity({ Parent.ChildrenIDs[0], this});

		if (Parent.Parent) 
		{
			Entity parent = { Parent.Parent, this };
			parent.GetParentChild().RemoveChild(entity.GetUUID());
		}

		m_SceneOrder.remove(entity.GetUUID());
		m_EntityMap.erase(entity.GetUUID());
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

	Entity Scene::GetEntityWithTag(std::string_view tag) {
		auto view = m_Registry.view<TagComponent>();

		for (auto& entity : view) {
			const TagComponent& tc = view.get<TagComponent>(entity);

			if (tc.Tag == tag)
				return Entity(entity, this);
		}

		NB_ERROR("Could Not Find Entity with name {0}", tag);
		return {};
	}

	Entity Scene::GetEntityWithUUID(UUID id) {
		bool found = m_EntityMap.find(id) != m_EntityMap.end();
		NB_ASSERT(found, "Could Not Find Entity UUID");

		return { m_EntityMap[id], this };
	}

	void Scene::CreateBox2DBody(Entity entity) {
		auto& world = entity.GetComponent<WorldTransformComponent>();
		auto& transform = entity.GetComponent<TransformComponent>();
		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

		vec3 translation, rotation, scale;
		DecomposeTransform(world.Transform, translation, rotation, scale);

		b2BodyDef bodyDef;
		bodyDef.type = Rigibody2DToBox2D(rb2d.Type);
		bodyDef.position.Set(translation.x, translation.y);
		bodyDef.angle = rotation.z;
		bodyDef.fixedRotation = rb2d.FixedRotation;

		b2BodyUserData data;
		data.pointer = reinterpret_cast<uintptr_t>(new Entity(entity, this));
		bodyDef.userData = data;

		b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
		rb2d.RuntimeBody = body;

		if (entity.HasComponent<BoxCollider2DComponent>()) {
			auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

			b2PolygonShape polygonShape;
			polygonShape.SetAsBox(scale.x * bc2d.Size.x, scale.y * bc2d.Size.y, b2Vec2(bc2d.Offset.x, bc2d.Offset.y), 0.0f);

			b2FixtureDef fixtureDef;
			fixtureDef.shape = &polygonShape;
			fixtureDef.density = bc2d.Density;
			fixtureDef.friction = bc2d.Friction;
			fixtureDef.restitution = bc2d.Restitution;
			fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;

			fixtureDef.filter.categoryBits = bc2d.Category;
			fixtureDef.filter.maskBits = bc2d.Mask;
			
			bc2d.RuntimeFixture = body->CreateFixture(&fixtureDef);
		}

		if (entity.HasComponent<CircleColliderComponent>()) {
			auto& cc = entity.GetComponent<CircleColliderComponent>();

			b2CircleShape circle;
			circle.m_p.Set(cc.Offset.x, cc.Offset.y);
			circle.m_radius = cc.Radius * scale.x;

			b2FixtureDef fixtureDef;
			fixtureDef.shape = &circle;
			fixtureDef.density = cc.Density;
			fixtureDef.friction = cc.Friction;
			fixtureDef.restitution = cc.Restitution;
			fixtureDef.restitutionThreshold = cc.RestitutionThreshold;

			fixtureDef.filter.categoryBits = cc.Category;
			fixtureDef.filter.maskBits = cc.Mask;

			cc.RuntimeFixture = body->CreateFixture(&fixtureDef);
		}
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
			
			auto& world = entity.GetComponent<WorldTransformComponent>();
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			if (!rb2d.RuntimeBody)
				CreateBox2DBody(entity);

			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			auto position = body->GetPosition();

			vec3 wTranslation, wRotation, wScale;
			DecomposeTransform(world.Transform, wTranslation, wRotation, wScale);
			
			transform.Translation.x += position.x - wTranslation.x;
			transform.Translation.y += position.y - wTranslation.y;
			transform.Rotation.z += body->GetAngle() - wRotation.z;

			UpdateChildrenAndTransform(entity);
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

	void Scene::InitScripts() {
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, NativeScriptComponent& nsc) {
			nsc.Instance = nsc.InstantiateScript();
			nsc.Instance->m_Entity = Entity{ entity, this };
			nsc.Instance->m_Scene = this;
			nsc.Instance->Start();
		});

		ScriptEngine::OnRuntimeStart(this);

		auto view = m_Registry.view<ScriptComponent>();
		for (auto e : view) {
			ScriptEngine::OnCreateEntity({ e, this });
		}
	}

	void Scene::UpdateScripts() {
		auto scriptView = m_Registry.view<ScriptComponent>();
		for (auto e : scriptView)
			ScriptEngine::OnUpdateEntity({ e, this }, Time::DeltaTime());
		
		auto nativeScriptView = m_Registry.view<NativeScriptComponent>();
		for (auto e : nativeScriptView)
		{
			auto& nsc = nativeScriptView.get<NativeScriptComponent>(e);

			if (!nsc.Instance) 
			{
				nsc.Instance = nsc.InstantiateScript();
				nsc.Instance->m_Entity = Entity{ e, this };
				nsc.Instance->m_Scene = this;
				nsc.Instance->Start();
			}

			nsc.Instance->Update();
		}
	}

	void Scene::DestroyScripts() {
		ScriptEngine::OnRuntimeStop();

		auto view = m_Registry.view<NativeScriptComponent>();
		for (auto e : view) 
		{
			auto& nsc = view.get<NativeScriptComponent>(e);
			if (!nsc.Instance)
				return;

			nsc.Instance->Destroy();
			nsc.DestroyScript(&nsc);
		}
	}

	void Scene::OnRuntimeStart() {
		m_IsRunning = true;

		InitPhysics();
		InitScripts();
	}

	void Scene::OnRuntimeStop() {
		m_IsRunning = false;

		DestroyScripts();
		DestroyPhysics();
	}

	void Scene::UpdateRuntime() {
		if (m_IsPaused && m_StepFrames-- <= 0)
			return;

		auto camView = m_Registry.view<TransformComponent, CameraComponent>();
		for (auto entity : camView) {
			auto [transform, camera] = camView.get<TransformComponent, CameraComponent>(entity);

			if (camera.Primary) {
				mainCam = &camera.Camera;
				mainCamTransform = transform;
				break;
			}
		}

		UpdateScripts();
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
		if (m_IsPaused && m_StepFrames-- <= 0)
			return;

		UpdatePhysics();
	}

	void Scene::Step(int frames)
	{
		if (m_StepFrames > 0)
			frames += m_StepFrames;

		m_StepFrames = frames;
	}

	void Scene::UpdateEditor() { }

	void Scene::Render(EditorCamera& camera) {
		Renderer2D::BeginScene(camera);

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

	void Scene::RenderOverlay(EditorCamera& camera) {
		Renderer2D::BeginScene(camera);

		auto StringGroup = m_Registry.view<StringRendererComponent>();
		for (auto entity : StringGroup) {
			Renderer2D::Draw(NB_STRING, Entity{ entity, this });
		}

		Renderer2D::EndScene();
	}

	void Scene::Render(const Camera& camera, const mat4& transform) {
		Renderer2D::BeginScene(camera, transform);

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

	void Scene::RenderOverlay(const Camera& camera, const mat4& transform) {
		Renderer2D::BeginScene(camera, transform);

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
		static_assert(sizeof(T) == 0);
	}
	
	template<>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component) { }

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component) { }

	template<>
	void Scene::OnComponentAdded<ParentChildComponent>(Entity entity, ParentChildComponent& component) { }
	
	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component) { }
	
	template<>
	void Scene::OnComponentAdded<WorldTransformComponent>(Entity entity, WorldTransformComponent& component) { }

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component) {
		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			component.Camera.SetViewPortSize(m_ViewportWidth, m_ViewportHeight);
	}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component) { }

	template<>
	void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component) { }

	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component) { }

	template<>
	void Scene::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent& component) { }

	template<>
	void Scene::OnComponentAdded<StringRendererComponent>(Entity entity, StringRendererComponent& component) { component.InitiateFont(); }

	template<>
	void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component) {
		if (!m_IsRunning)
			return;
		
		//Future: Update Body
		if (b2Body* body = (b2Body*)entity.GetComponent<Rigidbody2DComponent>().RuntimeBody)
			m_PhysicsWorld->DestroyBody(body);

		CreateBox2DBody(entity);
	}

	template<>
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component) {
		if (!m_IsRunning || !entity.HasComponent<Rigidbody2DComponent>())
			return;
	
		//Future: Update Body
		if (b2Body* body = (b2Body*)entity.GetComponent<Rigidbody2DComponent>().RuntimeBody)
			m_PhysicsWorld->DestroyBody(body);
		
		CreateBox2DBody(entity);
	}

	template<>
	void Scene::OnComponentAdded<CircleColliderComponent>(Entity entity, CircleColliderComponent& component) {
		if (!m_IsRunning || !entity.HasComponent<Rigidbody2DComponent>())
			return;

		//Future: Update Body
		if (b2Body* body = (b2Body*)entity.GetComponent<Rigidbody2DComponent>().RuntimeBody)
			m_PhysicsWorld->DestroyBody(body);

		CreateBox2DBody(entity);
	}
}