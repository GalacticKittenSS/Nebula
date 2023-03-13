#pragma once

#include "Entity.h"

namespace Nebula
{
	class PrefabSerializer
	{
	public:
		PrefabSerializer(Scene* scene);

		void Serialize(Entity entity, const std::string& filepath);
		Entity Deserialize(const std::string& filepath);
	private:
		Scene* m_Scene;
	};
}