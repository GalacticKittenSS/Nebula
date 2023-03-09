#include "nbpch.h"
#include "Entity.h"

#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"

namespace Nebula {
	void Entity::CalculateTransform() {
		auto& world = GetComponent<WorldTransformComponent>();
		glm::mat4 transform = GetTransform().CalculateMatrix();
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

		WorldTransformComponent& transform = GetComponent<WorldTransformComponent>();

		glm::vec3 translation, rotation, scale;
		Maths::DecomposeTransform(transform.Transform, translation, rotation, scale);
		
		if (b2Body* body = (b2Body*)GetComponent<Rigidbody2DComponent>().RuntimeBody)
			body->SetTransform({ translation.x, translation.y }, rotation.z);
		
		if (HasComponent<BoxCollider2DComponent>())
		{
			BoxCollider2DComponent bc2d = GetComponent<BoxCollider2DComponent>();
			if (b2Fixture* fixture = (b2Fixture*)bc2d.RuntimeFixture)
			{
				b2PolygonShape* shape = (b2PolygonShape*)fixture->GetShape();
				shape->SetAsBox(scale.x * bc2d.Size.x, scale.y * bc2d.Size.y, 
					b2Vec2(bc2d.Offset.x, bc2d.Offset.y), 0.0f);
			}
		}

		if (HasComponent<CircleColliderComponent>())
		{
			CircleColliderComponent cc = GetComponent<CircleColliderComponent>();
			if (b2Fixture* fixture = (b2Fixture*)cc.RuntimeFixture)
			{
				b2CircleShape* circle = (b2CircleShape*)fixture->GetShape();
				circle->m_radius = cc.Radius * scale.x;
			}
		}
	}

	void Rigidbody2DComponent::ApplyAngularImpulse(float impulse) {
		((b2Body*)RuntimeBody)->ApplyAngularImpulse(impulse, true);
	}

	void Rigidbody2DComponent::ApplyForce(glm::vec2 force, glm::vec2 point) {
		((b2Body*)RuntimeBody)->ApplyForce({ force.x, force.y }, { point.x, point.y }, true);
	}

	void Rigidbody2DComponent::ApplyLinearImpulse(glm::vec2 impulse, glm::vec2 point) {
		((b2Body*)RuntimeBody)->ApplyLinearImpulse({ impulse.x, impulse.y }, { point.x, point.y }, true);
	}

	void Rigidbody2DComponent::ApplyTorque(float torque) {
		((b2Body*)RuntimeBody)->ApplyTorque(torque, true);
	}

	void Rigidbody2DComponent::ApplyForceToCenter(glm::vec2 force) {
		((b2Body*)RuntimeBody)->ApplyForceToCenter({ force.x, force.y }, true);
	}

	void Rigidbody2DComponent::ApplyLinearImpulseToCenter(glm::vec2 impulse) {
		((b2Body*)RuntimeBody)->ApplyLinearImpulseToCenter({ impulse.x, impulse.y }, true);
	}

	static void UpdateFixtureFilters(b2Fixture* fixture, uint16_t category, uint16_t mask) {
		b2Filter filter;
		filter.categoryBits = category;
		filter.maskBits = mask;

		fixture->SetFilterData(filter);
	}

	// TODO: Move Functions Elsewhere
	void BoxCollider2DComponent::UpdateFilters(uint16_t category, uint16_t mask) {
		//Category = (Rigidbody2DComponent::Filters)category;
		//Mask = mask;

		if (!(b2Fixture*)RuntimeFixture)
			return;

		UpdateFixtureFilters((b2Fixture*)RuntimeFixture, category, mask);
	}

	void CircleColliderComponent::UpdateFilters(uint16_t category, uint16_t mask) {
		//Category = category;
		//Mask = mask;

		if (!(b2Fixture*)RuntimeFixture)
			return;

		UpdateFixtureFilters((b2Fixture*)RuntimeFixture, category, mask);
	}
}