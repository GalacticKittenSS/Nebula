#include "nbpch.h"
#include "Scene.h"

#include "Nebula/Renderer/Renderer2D.h"
#include "Nebula/Scripting/ScriptEngine.h"
#include "Nebula/Utils/Time.h"
#include "Nebula/Utils/Physics2D.h"
#include "Nebula/Project/Project.h"

#include "Components.h"
#include "Entity.h"
#include "ContactListener.h"
#include "SceneRenderer.h"

#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"

namespace Nebula {
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
		NB_PROFILE_FUNCTION();

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

		newScene->m_Nodes = other->m_Nodes;
		newScene->m_SceneOrder = other->m_SceneOrder;

		return newScene;
	}

	Entity Scene::CreateEntity(std::string_view name) {
		return CreateEntity(UUID(), name);
	}

	Entity Scene::CreateEntity(UUID uuid, std::string_view name) {
		NB_PROFILE_FUNCTION();

		Entity entity = { m_Registry.create(), this };
		m_SceneOrder.push_back(uuid); 
		m_EntityMap[uuid] = entity;
		m_Nodes[uuid] = SceneNode(uuid);

		entity.AddComponent<TransformComponent>();
		entity.AddComponent<WorldTransformComponent>();
		entity.AddComponent<MaterialComponent>();

		auto& idc = entity.AddComponent<IDComponent>();
		idc.ID = uuid;
		
		const ProjectConfig& pConfig = Project::GetActive()->GetConfig();

		auto& prop = entity.AddComponent<PropertiesComponent>();
		prop.Layer = pConfig.Layers.at(1);

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		
		return entity;
	}

	Entity Scene::DuplicateEntity(Entity entity, bool duplicateIntoParent) {
		NB_PROFILE_FUNCTION();

		std::string name = entity.GetName();
		Entity duplicated = CreateEntity(name);

		CopyComponent(AllComponents{}, duplicated, entity);

		const UUID& duplicatedID = duplicated.GetUUID();

		auto& pcc = m_Nodes.at(entity.GetUUID());
		if (pcc.Parent && duplicateIntoParent)
		{
			m_Nodes.at(duplicatedID).Parent = pcc.Parent;
			m_Nodes.at(pcc.Parent).Children.push_back(duplicatedID);
			m_SceneOrder.remove(duplicatedID);
		}
		
		for (uint32_t i = 0; i < pcc.Children.size(); i++)
		{
			Entity child = DuplicateEntity({ pcc.Children[i], this }, false);
			UUID childID = child.GetUUID();

			m_Nodes.at(duplicatedID).Children.push_back(childID);
			m_Nodes.at(childID).Parent = duplicatedID;
			m_SceneOrder.remove(childID);
		}
		
		if (m_IsRunning && entity.HasComponent<Rigidbody2DComponent>())
			CreateBox2DBody(duplicated);
		
		if (entity.HasComponent<ScriptComponent>())
		{
			ScriptEngine::CreateScriptInstance(duplicated);
			ScriptEngine::CopyScriptFields(entity.GetUUID(), duplicatedID);

			if (m_IsRunning)
				ScriptEngine::OnCreateEntity(duplicated);
		}
		
		return duplicated;
	}

	void Scene::DestroyEntity(Entity entity) {
		NB_PROFILE_FUNCTION();

		if (!entity)
			return;
		
		UUID entityID = entity.GetUUID();

		if (m_ContactListener && m_ContactListener->IsFlushing())
		{
			m_ContactListener->DeleteEntity(entityID);
			return;
		}

		if (entity.HasComponent<NativeScriptComponent>())
		{
			auto nsc = entity.GetComponent<NativeScriptComponent>();
			entity.GetComponent<NativeScriptComponent>().DestroyScript(&nsc);
		}

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			Rigidbody2DComponent& rb2d = entity.GetComponent<Rigidbody2DComponent>();
			if (b2Body* body = (b2Body*)rb2d.RuntimeBody)
			{
				Entity* entity = (Entity*)body->GetUserData().pointer;
				
				m_PhysicsWorld->DestroyBody(body);
				rb2d.RuntimeBody = nullptr;

				delete entity;
			}
		}
		
		auto& Parent = m_Nodes.at(entityID);
		size_t size = Parent.Children.size();
		for (size_t i = 0; i < size; i++)
			DestroyEntity({ Parent.Children[0], this});

		if (Parent.Parent) 
		{
			auto& parent = m_Nodes.at(Parent.Parent);
			parent.Children.remove(entityID);
		}

		ScriptEngine::DeleteScriptInstance(entityID);

		m_Nodes.erase(entityID);
		m_SceneOrder.remove(entityID);
		m_EntityMap.erase(entityID);
		m_Registry.destroy(entity);
	}

	Entity Scene::GetPrimaryCamera() {
		NB_PROFILE_FUNCTION();

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
		NB_PROFILE_FUNCTION();

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
		auto it = m_EntityMap.find(id);
		NB_ASSERT(it != m_EntityMap.end(), "Could Not Find Entity UUID");

		return { it->second, this };
	}

	Scene::SceneNode& Scene::GetEntityNode(UUID entityID) {
		auto it = m_Nodes.find(entityID);
		NB_ASSERT(it != m_Nodes.end(), "Could Not Find Entity Node");

		return it->second;
	}

	void Scene::CreateBox2DBody(Entity entity) {
		NB_PROFILE_FUNCTION();

		auto& world = entity.GetComponent<WorldTransformComponent>();
		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		auto& prop = entity.GetComponent<PropertiesComponent>();

		glm::vec3 translation, rotation, scale;
		Maths::DecomposeTransform(world.Transform, translation, rotation, scale);

		b2BodyDef bodyDef;
		bodyDef.type = Utils::Rigibody2DToBox2D(rb2d.Type);
		bodyDef.position.Set(translation.x, translation.y);
		bodyDef.angle = rotation.z;
		bodyDef.fixedRotation = rb2d.FixedRotation;

		b2BodyUserData data;
		data.pointer = reinterpret_cast<uintptr_t>(new UUID(entity.GetUUID()));
		bodyDef.userData = data;

		b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
		body->SetEnabled(prop.Enabled);
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

			fixtureDef.isSensor = rb2d.Trigger;
			fixtureDef.filter.categoryBits = prop.Layer->Identity;
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

			fixtureDef.isSensor = rb2d.Trigger;
			fixtureDef.filter.categoryBits = prop.Layer->Identity;
			fixtureDef.filter.maskBits = cc.Mask;

			cc.RuntimeFixture = body->CreateFixture(&fixtureDef);
		}
	}

	void Scene::UpdateBox2DBody(Entity entity)
	{
		NB_PROFILE_FUNCTION();

		if (!m_IsRunning)
			return;

		auto& world = entity.GetComponent<WorldTransformComponent>();
		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		auto& prop = entity.GetComponent<PropertiesComponent>();

		glm::vec3 translation, rotation, scale;
		Maths::DecomposeTransform(world.Transform, translation, rotation, scale);

		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->SetEnabled(prop.Enabled);
		body->SetType(Utils::Rigibody2DToBox2D(rb2d.Type));
		body->SetTransform({ translation.x, translation.y }, rotation.z);
		body->SetFixedRotation(rb2d.FixedRotation);

		if (entity.HasComponent<BoxCollider2DComponent>()) {
			auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

			if (!bc2d.RuntimeFixture)
			{
				b2PolygonShape polygonShape;
				polygonShape.SetAsBox(scale.x * bc2d.Size.x, scale.y * bc2d.Size.y, b2Vec2(bc2d.Offset.x, bc2d.Offset.y), 0.0f);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &polygonShape;

				bc2d.RuntimeFixture = body->CreateFixture(&fixtureDef);
			}

			b2Fixture* fixture = (b2Fixture*)bc2d.RuntimeFixture;
			fixture->SetDensity(bc2d.Density);
			fixture->SetFriction(bc2d.Friction);
			fixture->SetRestitution(bc2d.Restitution);
			fixture->SetRestitutionThreshold(bc2d.RestitutionThreshold);
			fixture->SetSensor(rb2d.Trigger);
			
			b2PolygonShape* polygonShape = (b2PolygonShape*)fixture->GetShape();;
			polygonShape->SetAsBox(scale.x * bc2d.Size.x, scale.y * bc2d.Size.y, b2Vec2(bc2d.Offset.x, bc2d.Offset.y), 0.0f);

			b2Filter filter;
			filter.categoryBits = prop.Layer->Identity;
			filter.maskBits = bc2d.Mask;
			fixture->SetFilterData(filter);
		}

		if (entity.HasComponent<CircleColliderComponent>()) {
			auto& cc = entity.GetComponent<CircleColliderComponent>();

			if (!cc.RuntimeFixture)
			{
				b2CircleShape circle;
				circle.m_p.Set(cc.Offset.x, cc.Offset.y);
				circle.m_radius = cc.Radius * scale.x;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &circle;

				cc.RuntimeFixture = body->CreateFixture(&fixtureDef);
			}

			b2Fixture* fixture = (b2Fixture*)cc.RuntimeFixture;
			fixture->SetDensity(cc.Density);
			fixture->SetFriction(cc.Friction);
			fixture->SetRestitution(cc.Restitution);
			fixture->SetRestitutionThreshold(cc.RestitutionThreshold);
			fixture->SetSensor(rb2d.Trigger);

			b2CircleShape* circle = (b2CircleShape*)fixture->GetShape();
			circle->m_p.Set(cc.Offset.x, cc.Offset.y);
			circle->m_radius = cc.Radius * scale.x;

			b2Filter filter;
			filter.categoryBits = prop.Layer->Identity;
			filter.maskBits = cc.Mask;
			fixture->SetFilterData(filter);
		}

		body->ResetMassData();
	}

	void Scene::InitPhysics() {
		NB_PROFILE_FUNCTION();

		m_ContactListener = new ContactListener(this);

		glm::vec2 gravity = Project::GetActive()->GetConfig().Gravity;
		
		m_PhysicsWorld = new b2World({ gravity.x, gravity.y });
		m_PhysicsWorld->SetAllowSleeping(false);
		m_PhysicsWorld->SetContactListener(m_ContactListener);

		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view)
			CreateBox2DBody(Entity{ e, this });
	}

	void Scene::UpdatePhysics() {
		NB_PROFILE_FUNCTION();

		m_PhysicsWorld->Step(Time::DeltaTime(), 6, 2);
		m_ContactListener->Flush();

		auto view = m_Registry.view<WorldTransformComponent, TransformComponent, Rigidbody2DComponent>();

		for (auto e : view) {
			NB_PROFILE_SCOPE("Scene::UpdatePhysics - Update Body");
			auto& [world, transform, rb2d] = view.get<WorldTransformComponent, TransformComponent, Rigidbody2DComponent>(e);
			
			Entity entity = { e, this };
			if (!rb2d.RuntimeBody)
				CreateBox2DBody(entity);

			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			const auto& position = body->GetPosition();

			glm::vec3 wTranslation, wRotation, wScale;
			Maths::DecomposeTransform(world.Transform, wTranslation, wRotation, wScale);
			
			transform.Translation.x += position.x - wTranslation.x;
			transform.Translation.y += position.y - wTranslation.y;
			transform.Rotation.z += body->GetAngle() - wRotation.z;

			entity.UpdateTransform(false);
		}
	}

	void Scene::DestroyPhysics() {
		NB_PROFILE_FUNCTION();

		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view) {
			Entity entity = { e, this };
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
			if (rb2d.RuntimeBody) {
				if (Entity* entity = (Entity*)((b2Body*)rb2d.RuntimeBody)->GetUserData().pointer)
					delete entity;
				rb2d.RuntimeBody = nullptr;
			}
		}

		delete m_PhysicsWorld; m_PhysicsWorld = nullptr;
		delete m_ContactListener; m_ContactListener = nullptr;
	}

	void Scene::InitScripts() {
		NB_PROFILE_FUNCTION();

		m_Registry.view<NativeScriptComponent>().each([=](auto entity, NativeScriptComponent& nsc) {
			nsc.Instance = nsc.InstantiateScript();
			nsc.Instance->m_Entity = Entity{ entity, this };
			nsc.Instance->m_Scene = this;
			nsc.Instance->Start();
		});

		ScriptEngine::OnRuntimeStart(this);

		auto view = m_Registry.view<ScriptComponent>();
		
		// TODO: Remove (CreateScriptInstance should be called automatically)
		// Make sure all entities have a script instance before calling OnCreate
		// Not doing so may cause a crash when call Entity.As in C#
		for (auto e : view)
		{
			Entity entity = { e, this };
			ScriptEngine::CreateScriptInstance(entity);
		}

		for (auto e : view)
		{
			Entity entity = { e, this };
			if (entity.IsEnabled())
				ScriptEngine::OnCreateEntity(entity);
		}
	}

	void Scene::UpdateScripts() {
		NB_PROFILE_FUNCTION();

		auto scriptView = m_Registry.view<ScriptComponent>();
		for (auto e : scriptView)
		{
			Entity entity = { e, this };
			if (entity.IsEnabled())
				ScriptEngine::OnUpdateEntity(entity, Time::DeltaTime());
		}
		
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
		NB_PROFILE_FUNCTION();

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
		NB_PROFILE_FUNCTION();

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

	void Scene::OnSimulationStart() {
		InitPhysics();
	}

	void Scene::OnSimulationStop() {
		DestroyPhysics();
	}

	void Scene::UpdateSimulation() {
		NB_PROFILE_FUNCTION();

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

	void Scene::OnViewportResize(uint32_t width, uint32_t height) {
		NB_PROFILE_FUNCTION();

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
	void Scene::OnComponentAdded(Entity entity, T& component) { }
	
	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component) {
		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			component.Camera.SetViewPortSize(m_ViewportWidth, m_ViewportHeight);
	}

	template<>
	void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component) {
		if (!m_IsRunning)
			return;

		CreateBox2DBody(entity);
	}

	template<>
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component) {
		if (!m_IsRunning || !entity.HasComponent<Rigidbody2DComponent>())
			return;

		UpdateBox2DBody(entity);
	}

	template<>
	void Scene::OnComponentAdded<CircleColliderComponent>(Entity entity, CircleColliderComponent& component) {
		if (!m_IsRunning || !entity.HasComponent<Rigidbody2DComponent>())
			return;

		UpdateBox2DBody(entity);
	}
}