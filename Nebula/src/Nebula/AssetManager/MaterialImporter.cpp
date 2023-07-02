#include "nbpch.h"
#include "MaterialImporter.h"

namespace Nebula
{
	Ref<Material> MaterialImporter::ImportMaterial(AssetHandle handle, const AssetMetadata& metadata)
	{
		Ref<Material> mat = CreateRef<Material>();
		
		MaterialSerializer serializer(mat);
		serializer.Deserialize(metadata.Path.string());

		return mat;
	}
}