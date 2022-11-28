#include "nbpch.h"
#include "Entity.h"

#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"

namespace Nebula {
	void Entity::CalculateTransform() {
		auto& world = GetComponent<WorldTransformComponent>();
		mat4 transform = GetTransform().CalculateMatrix();
		world.Transform = transform;
		
		UUID parentID = GetParentChild().Parent;
		if (parentID) {
			Entity parent = { parentID, m_Scene };
			parent.CalculateTransform();
			auto& pWorld = parent.GetComponent<WorldTransformComponent>();
			world.Transform = pWorld.Transform * transform;
		}
	}

	void Entity::UpdateTransform() {
		CalculateTransform();

		for (UUID id : GetParentChild().ChildrenIDs)
		{
			Entity child = { id, m_Scene };
			child.UpdateTransform();
		}

		UpdatePhysicsBody();
	}

	void Entity::UpdatePhysicsBody()
	{
		if (!HasComponent<Rigidbody2DComponent>())
			return;

		if (m_Scene->m_PhysicsWorld && m_Scene->m_PhysicsWorld->IsLocked())
		{
			m_Scene->m_BodiesToUpdate.push_back(GetUUID());
		}
		else if (b2Body* body = (b2Body*)GetComponent<Rigidbody2DComponent>().RuntimeBody)
		{
			WorldTransformComponent& transform = GetComponent<WorldTransformComponent>();

			vec3 translation, rotation, scale;
			DecomposeTransform(transform.Transform, translation, rotation, scale);
			body->SetTransform({ translation.x, translation.y }, rotation.z);
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

	static void UpdateFixtureFilters(b2Fixture* fixture, uint16_t category, uint16_t mask) {
		b2Filter filter;
		filter.categoryBits = category;
		filter.maskBits = mask;

		fixture->SetFilterData(filter);
	}

	void BoxCollider2DComponent::UpdateFilters(uint16_t category, uint16_t mask) {
		Category = (Rigidbody2DComponent::Filters)category;
		Mask = mask;

		if (!(b2Fixture*)RuntimeFixture)
			return;

		UpdateFixtureFilters((b2Fixture*)RuntimeFixture, category, mask);
	}

	void CircleColliderComponent::UpdateFilters(uint16_t category, uint16_t mask) {
		Category = category;
		Mask = mask;

		if (!(b2Fixture*)RuntimeFixture)
			return;

		UpdateFixtureFilters((b2Fixture*)RuntimeFixture, category, mask);
	}
}