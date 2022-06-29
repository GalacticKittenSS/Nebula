#include "nbpch.h"
#include "Entity.h"

#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"

namespace Nebula {
	void CalculateGlobalTransform(Entity& entity) {
		auto& world = entity.GetComponent<WorldTransformComponent>();
		mat4& transform = entity.GetTransform().CalculateMatrix();
		world.Transform = transform;
		
		UUID parentID = entity.GetParentChild().Parent;
		if (parentID) {
			Entity parent = { parentID, entity };
			CalculateGlobalTransform(parent);
			auto& pWorld = parent.GetComponent<WorldTransformComponent>();
			world.Transform = pWorld.Transform * transform;
		}
	}

	void UpdateChildrenAndTransform(Entity& entity) {
		CalculateGlobalTransform(entity);

		for (UUID child : entity.GetParentChild().ChildrenIDs) {
			Entity c = { child, entity };
			if (!c.GetParentChild().ChildrenIDs.size())
				CalculateGlobalTransform(c);
			else
				UpdateChildrenAndTransform(c);
		}

		if (entity.HasComponent<Rigidbody2DComponent>()) {
			Rigidbody2DComponent& rb2d = entity.GetComponent<Rigidbody2DComponent>();
			if (rb2d.RuntimeBody) {
				WorldTransformComponent& transform = entity.GetComponent<WorldTransformComponent>();

				b2Body* body = (b2Body*)rb2d.RuntimeBody;

				vec3 translation, rotation, scale;
				DecomposeTransform(transform.Transform, translation, rotation, scale);
				body->SetTransform({ translation.x, translation.y }, rotation.z);
			}
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