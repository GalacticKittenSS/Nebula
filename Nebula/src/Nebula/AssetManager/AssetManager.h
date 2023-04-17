#pragma once

#include "Nebula/Project/Project.h"

namespace Nebula
{
	class AssetManager
	{
	public:
		static inline AssetHandle ImportAsset(const std::filesystem::path& path)
		{
			NB_ASSERT(Project::GetAssetManager());
			return Project::GetAssetManager()->ImportAsset(path);
		}

		static inline AssetHandle GetHandleFromPath(const std::filesystem::path& path)
		{
			NB_ASSERT(Project::GetAssetManager());
			return Project::GetAssetManager()->GetHandleFromPath(path);
		}

		static inline Ref<Asset> GetAsset(AssetHandle handle, bool load = true)
		{
			NB_ASSERT(Project::GetAssetManager());
			return Project::GetAssetManager()->GetAsset(handle, load);
		}

		static inline AssetType GetAssetType(AssetHandle handle)
		{
			NB_ASSERT(Project::GetAssetManager());
			return Project::GetAssetManager()->GetAssetType(handle);
		}

		template <typename T>
		static inline Ref<T> GetAssetData(AssetHandle handle)
		{
			NB_ASSERT(Project::GetAssetManager());
			return Project::GetAssetManager()->GetAssetData<T>(handle);
		}

		static inline Array<AssetHandle> GetAllAssetsWithType(AssetType type, bool global = false)
		{
			NB_ASSERT(Project::GetAssetManager());
			return Project::GetAssetManager()->GetAllAssetsWithType(type, global);
		}

		static inline void GetAllAssetsWithType(Array<AssetHandle>& handlesArray, AssetType type, bool global = false)
		{
			NB_ASSERT(Project::GetAssetManager());
			Project::GetAssetManager()->GetAllAssetsWithType(handlesArray, type, global);
		}

		static inline void ImportFontFamily(const std::filesystem::path& directory, const std::string& name)
		{
			AssetManagerBase::ImportFontFamily(directory, name);
		}

		static AssetType GetTypeFromExtension(const std::string& extension)
		{
			if (extension == ".nebula")			return AssetType::Scene;
			else if (extension == ".prefab")	return AssetType::Prefab;
			else if (extension == ".cs")		return AssetType::Script;
			else if (extension == ".ttf" || extension == ".TTF")  return AssetType::Font;
			else if (extension == ".png" || extension == ".jpeg") return AssetType::Texture;

			return AssetType::None;
		}
	};
}