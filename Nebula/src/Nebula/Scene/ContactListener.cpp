#include "nbpch.h"
#include "ContactListener.h"

#include "Entity.h"
#include "Nebula/Scripting/ScriptEngine.h"

#include "box2d/b2_contact.h"

namespace Nebula {
	ContactListener::ContactListener(Scene* scene) 
		: m_Scene(scene) 
	{
	}

	void ContactListener::BeginContact(b2Contact* contact) 
	{
 		UUID* a = (UUID*)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
		UUID* b = (UUID*)contact->GetFixtureB()->GetBody()->GetUserData().pointer;

		if (!a || !b)
			return;

		Contact c;
		c.EntityA = *a;
		c.EntityB = *b;

		m_Contacts.push_back(c);
	}

	void ContactListener::EndContact(b2Contact* contact) 
	{
		UUID* a = (UUID*)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
		UUID* b = (UUID*)contact->GetFixtureB()->GetBody()->GetUserData().pointer;

		if (!a || !b)
			return;

		Contact c;
		c.EntityA = *a;
		c.EntityB = *b;

		m_Contacts.push_back(c);
	}

	void ContactListener::Clear()
	{
		for (Contact& contact : m_Contacts)
		{
			if (contact.IsTypeEnter)
			{
				CallEntityEnter(contact.EntityA, contact.EntityB);
				CallEntityEnter(contact.EntityB, contact.EntityA);
			}
			else
			{
				CallEntityExit(contact.EntityA, contact.EntityB);
				CallEntityExit(contact.EntityB, contact.EntityA);
			}
		}

		m_Contacts.clear();
	}

	void ContactListener::CallEntityEnter(UUID a, UUID b)
	{
		Entity entity = { a, m_Scene };
		Entity other = { b, m_Scene };

		if (entity.HasComponent<NativeScriptComponent>())
		{
			if (ScriptableEntity* instance = entity.GetComponent<NativeScriptComponent>().Instance)
				instance->OnCollisionEnter(other);
		}

		if (entity.HasComponent<ScriptComponent>())
			ScriptEngine::OnCollisionEnter(entity, other);
	}

	void ContactListener::CallEntityExit(UUID a, UUID b) {
		Entity entity = { a, m_Scene };
		Entity other = { b, m_Scene };

		if (entity.HasComponent<NativeScriptComponent>())
		{
			if (ScriptableEntity* instance = entity.GetComponent<NativeScriptComponent>().Instance)
				instance->OnCollisionExit(other);
		}

		if (entity.HasComponent<ScriptComponent>())
			ScriptEngine::OnCollisionExit(entity, other);
	}
}