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
		c.IsTypeEnter = true;

		m_Contacts.push_back(c);
	}

	void ContactListener::EndContact(b2Contact* contact) 
	{
		UUID* a = (UUID*)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
		UUID* b = (UUID*)contact->GetFixtureB()->GetBody()->GetUserData().pointer;

		if (!a || !b)
			return;

		if (m_DeletingEntities)
		{
			CallEntityExit(*a, *b);
			CallEntityExit(*b, *a);
		}
		else
		{
			Contact c;
			c.EntityA = *a;
			c.EntityB = *b;
			c.IsTypeEnter = false;

			m_Contacts.push_back(c);
		}
	}

	void ContactListener::Flush()
	{
		m_FlushingContacts = true;

		for (uint32_t i = 0; i < m_Contacts.size(); i++)
		{
			Contact& contact = m_Contacts[i];

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

		m_FlushingContacts = false;
		m_Contacts.clear();
		
		m_DeletingEntities = true;

		for (const UUID& uuid : m_EntitiesToDelete)
		{
			Entity entity = { uuid, m_Scene };
			m_Scene->DestroyEntity(entity);
		}

		m_DeletingEntities = false;
		m_EntitiesToDelete.clear();
	}

	void ContactListener::DeleteEntity(UUID entity)
	{
		if (m_EntitiesToDelete.find(entity) != m_EntitiesToDelete.size())
			return;

		m_EntitiesToDelete.push_back(entity);
	}

	void ContactListener::CallEntityEnter(UUID a, UUID b)
	{
		Entity entity = { a, m_Scene };
		Entity other = { b, m_Scene };

		if (!entity || !other)
			return;

		if (entity.HasComponent<NativeScriptComponent>())
		{
			if (ScriptableEntity* instance = entity.GetComponent<NativeScriptComponent>().Instance)
				instance->OnCollisionEnter(other);
		}

		if (entity.HasComponent<ScriptComponent>())
			ScriptEngine::OnCollisionEnter(entity, other);
	}

	void ContactListener::CallEntityExit(UUID a, UUID b) 
	{
		Entity entity = { a, m_Scene };
		Entity other = { b, m_Scene };

		if (!entity || !other)
			return;

		if (entity.HasComponent<NativeScriptComponent>())
		{
			if (ScriptableEntity* instance = entity.GetComponent<NativeScriptComponent>().Instance)
				instance->OnCollisionExit(other);
		}

		if (entity.HasComponent<ScriptComponent>())
			ScriptEngine::OnCollisionExit(entity, other);
	}
}