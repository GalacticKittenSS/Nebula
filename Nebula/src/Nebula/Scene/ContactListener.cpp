#include "nbpch.h"
#include "ContactListener.h"

#include "box2d/b2_contact.h"

namespace Nebula {
	void ContactListener::BeginContact(b2Contact* contact) {
		Entity* a = (Entity*)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
		Entity* b = (Entity*)contact->GetFixtureB()->GetBody()->GetUserData().pointer;

		if (!a || !b)
			return;

		CallEntityEnter(a, b); CallEntityEnter(b, a);
	}

	void ContactListener::EndContact(b2Contact* contact) {
		Entity* a = (Entity*)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
		Entity* b = (Entity*)contact->GetFixtureB()->GetBody()->GetUserData().pointer;

		if (!a || !b)
			return;

		CallEntityExit(a, b); CallEntityExit(b, a);
	}

	void ContactListener::CallEntityEnter(Entity* entity, Entity* other) {
		if (!entity->HasComponent<NativeScriptComponent>() || !entity->GetComponent<NativeScriptComponent>().Instance)
			return;

		entity->GetComponent<NativeScriptComponent>().Instance->OnCollisionEnter(*other);
	}

	void ContactListener::CallEntityExit(Entity* entity, Entity* other) {
		if (!entity->HasComponent<NativeScriptComponent>() || !entity->GetComponent<NativeScriptComponent>().Instance)
			return;

		entity->GetComponent<NativeScriptComponent>().Instance->OnCollisionExit(*other);
	}
}