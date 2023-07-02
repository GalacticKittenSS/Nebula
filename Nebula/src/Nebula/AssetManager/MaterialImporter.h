#pragma once

#include "Nebula/Renderer/Material.h"

namespace Nebula
{
	class MaterialImporter
	{
	public:
		static Ref<Material> ImportMaterial(AssetHandle handle, const AssetMetadata& metadata);
	};
}