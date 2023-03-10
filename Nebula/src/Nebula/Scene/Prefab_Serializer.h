#pragma once

#include "Entity.h"

namespace Nebula
{
	class PrefabSerializer
	{
	public:
		PrefabSerializer(const Ref<Scene>& scene);

		void Serialize(Entity entity, const std::string& filepath);
		bool Deserialize(const std::string& filepath);
	private:
		Ref<Scene> m_Scene;
	};
}