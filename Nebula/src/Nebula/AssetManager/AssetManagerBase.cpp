#include "nbpch.h"
#include "AssetManagerBase.h"

#include "AssetData.h"

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
			asset->IsLoaded = data->IsLoaded;
#
			if (!data->IsLoaded)
			{
				delete data;
				return nullptr;
			}

			return data;
		}

		static FontAsset* LoadFont(Ref<Asset> asset, const std::string& name)
		{
			FontAsset* data = new FontAsset(name, asset->Path);
			asset->IsLoaded = data->IsLoaded;
			
			if (!data->IsLoaded)
			{
				delete data;
				return nullptr;
			}

			return data;
		}
	}

	std::unordered_map<AssetHandle, Ref<Asset>> AssetManagerBase::m_GlobalAssets = {};

	void AssetManagerBase::OnAssetChange(const std::string& path, const filewatch::Event change_type)
	{
		if (change_type != filewatch::Event::modified)
			return;

		AssetHandle handle = Project::GetAssetManager()->GetHandleFromPath(path);
		Ref<Asset> asset = Project::GetAssetManager()->m_Assets.at(handle);

		Application::Get().SubmitToMainThread([asset]() {
			Project::GetAssetManager()->LoadAsset(asset);
		});
	}

	void AssetManagerBase::ImportAsset(AssetHandle handle, const std::filesystem::path& path, const std::filesystem::path& relativePath)
	{
		if (m_Assets.find(handle) != m_Assets.end())
			return;

		AssetType type = Utils::GetTypeFromExtension(path.extension().string());
		if (type == AssetType::None)
			return;

		Ref<Asset> asset = CreateRef<Asset>();
		asset->Handle = handle;
		asset->Path = path;
		asset->RelativePath = relativePath;
		asset->Type = type;

		if (type == AssetType::Font)
			asset->Data = Utils::LoadFont(asset, relativePath.string());

		m_Assets[handle] = asset;
	}

	AssetHandle AssetManagerBase::ImportAsset(const std::filesystem::path& path)
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

	AssetHandle AssetManagerBase::ImportFont(const std::string& name, const std::filesystem::path path, bool bold, bool italic)
	{
		if (!std::filesystem::exists(path))
			return NULL;

		AssetType type = Utils::GetTypeFromExtension(path.extension().string());
		if (type != AssetType::Font)
			return NULL;

		Ref<Asset> asset = CreateRef<Asset>();
		asset->Handle = AssetHandle();
		asset->Path = path;
		asset->RelativePath = path;
		asset->Type = type;

		FontAsset* data = Utils::LoadFont(asset, name + "_" + path.stem().string());
		data->Bold = bold;
		data->Italic = italic;
		asset->Data = data;

		m_GlobalAssets[asset->Handle] = asset;
		return asset->Handle;
	}

	void AssetManagerBase::ImportFontFamily(const std::filesystem::path& directory, std::string name)
	{
		std::filesystem::path path = directory / name;

		FontFamilyAsset* data = new FontFamilyAsset();
		data->Name = name;
		data->Regular = ImportFont(name, path / "Regular.ttf");
		data->Bold = ImportFont(name, path / "Bold.ttf");
		data->BoldItalic = ImportFont(name, path / "BoldItalic.ttf");
		data->Italic = ImportFont(name, path / "Italic.ttf");
		data->IsLoaded = true;

		Ref<Asset> asset = CreateRef<Asset>();
		asset->Handle = AssetHandle();
		asset->Path = directory;
		asset->Type = AssetType::FontFamily;
		asset->Data = data;
		asset->IsLoaded = true;

		m_GlobalAssets[asset->Handle] = asset;
	}

	AssetHandle AssetManagerBase::GetHandleFromPath(const std::filesystem::path& path)
	{
		for (const auto& [handle, asset] : m_Assets)
		{
			if (asset->Path == path)
				return handle;
		}

		for (const auto& [handle, asset] : m_GlobalAssets)
		{
			if (asset->Path == path)
				return handle;
		}

		return NULL;
	}

	bool AssetManagerBase::LoadAsset(Ref<Asset> asset)
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
			asset->Watcher = CreateScope<filewatch::FileWatch<std::string>>(asset->Path.string(), AssetManagerBase::OnAssetChange);
		
		return asset->IsLoaded;
	}

	Ref<Asset> AssetManagerBase::FindAsset(AssetHandle handle)
	{
		auto it = m_Assets.find(handle);
		if (it == m_Assets.end())
			return nullptr;

		return it->second;
	}

	Ref<Asset> AssetManagerBase::FindGlobalAsset(AssetHandle handle)
	{
		auto it = m_GlobalAssets.find(handle);
		if (it == m_GlobalAssets.end())
			return nullptr;

		return it->second;
	}

	Ref<Asset> AssetManagerBase::GetAsset(AssetHandle handle, bool load)
	{
		Ref<Asset> asset = FindAsset(handle);
		
		if (!asset)
			asset = FindGlobalAsset(handle);

		if (!asset)
			return nullptr;

		if (load && !asset->IsLoaded)
		{
			if (!LoadAsset(asset))
				return nullptr;
		}
		
		return asset;
	}

	AssetType AssetManagerBase::GetAssetType(AssetHandle handle)
	{
		auto it = m_Assets.find(handle);
		if (it == m_Assets.end())
			return AssetType::None;
		
		return it->second->Type;
	}

	Array<AssetHandle> AssetManagerBase::GetAllAssetsWithType(AssetType type, bool global)
	{
		Array<AssetHandle> handles;
		GetAllAssetsWithType(handles, type, global);
		return handles;
	}

	void AssetManagerBase::GetAllAssetsWithType(Array<AssetHandle>& handlesArray, AssetType type, bool global)
	{
		for (const auto& [handle, asset] : m_Assets)
		{
			if (asset->Type != type)
				continue;

			handlesArray.push_back(handle);
		}

		if (!global)
			return;

		for (const auto& [handle, asset] : m_GlobalAssets)
		{
			if (asset->Type != type)
				continue;

			handlesArray.push_back(handle);
		}
	}
}