#pragma once

#include "Entity.h"
#include "box2d/b2_world_callbacks.h"

namespace Nebula {
	class ContactListener : public b2ContactListener {
	public:
		void BeginContact(b2Contact* contact) override;
		void EndContact(b2Contact* contact) override;
	private:
		void CallEntityEnter(Entity* entity, Entity* other);
		void CallEntityExit(Entity* entity, Entity* other);
	};
}