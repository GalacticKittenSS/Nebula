#pragma once

#include "Nebula/Project/Project.h"

namespace Nebula
{
	class AssetManager
	{
	public:
		static inline AssetHandle CreateAsset(const std::filesystem::path& path)
		{
			NB_ASSERT(Project::GetAssetManager());
			return Project::GetAssetManager()->CreateAsset(path);
		}
		
		static AssetHandle CreateGlobalFamily(const std::filesystem::path& path)
		{
			std::string pathString = path.string();
			size_t lastSlash = pathString.find_last_of("/");
			std::string relativePath = pathString.substr(lastSlash + 1);

			AssetMetadata data;
			data.Type = AssetType::FontFamily;
			data.Path = path;
			data.RelativePath = relativePath; // family name
			data.isGlobal = true;

			if (!AssetManagerBase::CreateGlobalAsset(data))
				return NULL;

			return data.Handle;
		}

		static inline AssetHandle GetHandleFromPath(const std::filesystem::path& path)
		{
			NB_ASSERT(Project::GetAssetManager());
			return Project::GetAssetManager()->GetHandleFromPath(path);
		}

		template <typename T>
		static inline Ref<T> GetAsset(AssetHandle handle, bool load = true)
		{
			NB_ASSERT(Project::GetAssetManager());
			Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle, load);
			return std::static_pointer_cast<T>(asset);
		}

		static inline const AssetMetadata& GetAssetMetadata(AssetHandle handle)
		{
			NB_ASSERT(Project::GetAssetManager());
			return Project::GetAssetManager()->GetAssetMetadata(handle);
		}

		static inline Array<AssetHandle> GetAllAssetsWithType(AssetType type, bool global = true)
		{
			NB_ASSERT(Project::GetAssetManager());
			return Project::GetAssetManager()->GetAllAssetsWithType(type, global);
		}

		static inline void GetAllAssetsWithType(Array<AssetHandle>& handlesArray, AssetType type, bool global = true)
		{
			NB_ASSERT(Project::GetAssetManager());
			Project::GetAssetManager()->GetAllAssetsWithType(handlesArray, type, global);
		}

		static inline bool IsHandleValid(AssetHandle handle)
		{
			NB_ASSERT(Project::GetAssetManager());
			return Project::GetAssetManager()->IsHandleValid(handle);
		}
		
		static inline bool IsAssetLoaded(AssetHandle handle)
		{
			NB_ASSERT(Project::GetAssetManager());
			return Project::GetAssetManager()->IsAssetLoaded(handle);
		}

		static AssetType GetTypeFromExtension(std::string_view extension)
		{
			return Utils::GetTypeFromExtension(extension);
		}
	};
}