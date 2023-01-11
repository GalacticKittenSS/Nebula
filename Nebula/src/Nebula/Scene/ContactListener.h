#pragma once

#include "Nebula/Core/UUID.h"
#include "Scene.h"

#include "box2d/b2_world_callbacks.h"

namespace Nebula 
{
	class ContactListener : public b2ContactListener 
	{
	public:
		struct Contact
		{
			bool IsTypeEnter;
			UUID EntityA;
			UUID EntityB;
		};

		ContactListener(Scene* scene); 
		
		void BeginContact(b2Contact* contact) override;
		void EndContact(b2Contact* contact) override;

		void Flush();
		void DeleteEntity(UUID entity);

		inline bool IsFlushing() const { return m_FlushingContacts; }
	private:
		void CallEntityEnter(UUID entity, UUID other);
		void CallEntityExit(UUID entity, UUID other);

		Scene* m_Scene;
		Array<Contact> m_Contacts;
		Array<UUID> m_EntitiesToDelete;
		bool m_FlushingContacts = false, m_DeletingEntities = false;
	};
}