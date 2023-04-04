#include "nbpch.h"
#include "AssetManager.h"

#include "Nebula/Project/Project.h"
#include "Nebula/Core/Application.h"

namespace Nebula
{
	namespace Utils
	{
		AssetType GetTypeFromExtension(const std::string& extension) 
		{
			if (extension == ".nebula")			return AssetType::Scene;
			else if (extension == ".prefab")	return AssetType::Prefab;
			else if (extension == ".cs")		return AssetType::Script;
			else if (extension == ".ttf")		return AssetType::Font;
			else if (extension == ".png" || extension == ".jpeg") return AssetType::Texture;

			return AssetType::None;
		}
	}


	void AssetManager::OnAssetChange(const std::string& path, const filewatch::Event change_type)
	{
		if (change_type != filewatch::Event::modified)
			return;

		AssetHandle handle = Project::GetAssetManager()->GetHandleFromPath(path);
		Ref<Asset> asset = Project::GetAssetManager()->m_Assets.at(handle);

		Application::Get().SubmitToMainThread([asset]() {
			Project::GetAssetManager()->LoadAsset(asset);
		});
	}

	AssetHandle AssetManager::ImportAsset(const std::filesystem::path& path)
	{
		// Return asset if already loaded
		if (AssetHandle handle = GetHandleFromPath(path))
			return handle;
		
		AssetType type = Utils::GetTypeFromExtension(path.extension().string());
		if (type == AssetType::None)
			return 0;

		Ref<Asset> asset = CreateRef<Asset>();
		asset->Handle = AssetHandle();
		asset->Path = path;
		asset->Type = type;
		asset->Watcher = CreateScope<filewatch::FileWatch<std::string>>(path.string(), AssetManager::OnAssetChange);

		m_Assets[asset->Handle] = asset;
		return asset->Handle;
	}

	AssetHandle AssetManager::GetHandleFromPath(const std::filesystem::path& path)
	{
		for (const auto& [handle, asset] : m_Assets)
		{
			if (asset->Path == path)
				return handle;
		}

		return NULL;
	}

	bool AssetManager::LoadAsset(Ref<Asset> asset)
	{
		switch (asset->Type)
		{
		case AssetType::Texture:
			TextureAsset* data = new TextureAsset(asset->Path);
			if (data->Texture->IsLoaded())
			{
				asset->Data = data;
				asset->IsLoaded = true;
			}
			else
			{
				delete data;
			}

			break;
		}

		return asset->IsLoaded;
	}

	Ref<Asset> AssetManager::GetAsset(AssetHandle handle)
	{
		auto it = m_Assets.find(handle);
		if (it == m_Assets.end())
			return nullptr;

		Ref<Asset> asset = it->second;
		if (!asset->IsLoaded)
		{
			if (!LoadAsset(asset))
				return nullptr;
		}
		
		return asset;
	}

	AssetType AssetManager::GetAssetType(AssetHandle handle)
	{
		auto it = m_Assets.find(handle);
		if (it == m_Assets.end())
			return AssetType::None;
		
		return it->second->Type;
	}

	Array<AssetHandle> AssetManager::GetAllAssetsWithType(AssetType type)
	{
		Array<AssetHandle> handles;

		for (const auto& [handle, asset] : m_Assets)
		{
			if (asset->Type != type)
				continue;
			
			handles.push_back(handle);
		}

		return handles;
	}
}