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
			else if (extension == ".ttf" || extension == ".TTF")  return AssetType::Font;
			else if (extension == ".png" || extension == ".jpeg") return AssetType::Texture;

			return AssetType::None;
		}

		template <typename T>
		static T* Load(Ref<Asset> asset)
		{
			T* data = new T(asset->Path);
			if (!data->IsLoaded)
				delete data;

			asset->IsLoaded = data->IsLoaded;
			return data;
		}

		static FontAsset* LoadFont(Ref<Asset> asset, const std::string& name)
		{
			FontAsset* data = new FontAsset(name, asset->Path);
			if (!data->IsLoaded)
				delete data;

			asset->IsLoaded = data->IsLoaded;
			return data;
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

	AssetHandle AssetManager::ImportFont(const std::string& name, const std::filesystem::path& path)
	{
		// Return asset if already loaded
		if (AssetHandle handle = GetHandleFromPath(path))
			return handle;

		AssetType type = Utils::GetTypeFromExtension(path.extension().string());
		if (type != AssetType::Font)
			return 0;

		std::filesystem::path relativePath = std::filesystem::relative(path, Project::GetAssetDirectory());

		Ref<Asset> asset = CreateRef<Asset>();
		asset->Handle = AssetHandle();
		asset->Path = path;
		asset->RelativePath = relativePath;
		asset->Type = type;
		asset->Data = Utils::LoadFont(asset, name);

		m_Assets[asset->Handle] = asset;
		return asset->Handle;
	}

	AssetHandle AssetManager::ImportAsset(const std::filesystem::path& path)
	{
		// Return asset if already loaded
		if (AssetHandle handle = GetHandleFromPath(path))
			return handle;
		
		AssetType type = Utils::GetTypeFromExtension(path.extension().string());
		if (type == AssetType::None)
			return 0;

		std::filesystem::path relativePath = std::filesystem::relative(path, Project::GetAssetDirectory());

		Ref<Asset> asset = CreateRef<Asset>();
		asset->Handle = AssetHandle();
		asset->Path = path;
		asset->RelativePath = relativePath;
		asset->Type = type;
		
		if (type == AssetType::Font)
			asset->Data = Utils::LoadFont(asset, relativePath.string());

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
			asset->Data = Utils::Load<TextureAsset>(asset);
			break;
		case AssetType::Font:
			asset->Data = Utils::Load<FontAsset>(asset);
			break;
		}

		if (asset->IsLoaded)
			asset->Watcher = CreateScope<filewatch::FileWatch<std::string>>(asset->Path.string(), AssetManager::OnAssetChange);
		
		return asset->IsLoaded;
	}

	Ref<Asset> AssetManager::GetAsset(AssetHandle handle, bool load)
	{
		auto it = m_Assets.find(handle);
		if (it == m_Assets.end())
			return nullptr;

		Ref<Asset> asset = it->second;
		if (load && !asset->IsLoaded)
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