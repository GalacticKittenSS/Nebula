#include "nbpch.h"
#include "AssetManagerBase.h"

#include "AssetImporter.h"

#include "Nebula/Project/Project.h"
#include "Nebula/Core/Application.h"
#include "Nebula/Renderer/Texture.h"
#include "Nebula/Renderer/Fonts.h"

#include "Nebula/Utils/YAML.h"

namespace Nebula
{
	namespace Utils
	{
		std::string AssetTypeToString(AssetType type)
		{
			switch (type)
			{
			case AssetType::Scene: return "Scene";
			case AssetType::Prefab: return "Prefab";
			case AssetType::Texture: return "Texture";
			case AssetType::Font: return "Font";
			case AssetType::FontFamily: return "FontFamily";
			case AssetType::Script: return "Script";
			case AssetType::Material: return "Material";
			case AssetType::MemoryAsset: return "Memory";
			}

			return "None";
		}

		AssetType AssetTypeFromString(std::string_view type)
		{
			if (type == "Scene") return AssetType::Scene;
			else if (type == "Prefab") return AssetType::Prefab;
			else if (type == "Texture") return AssetType::Texture;
			else if (type == "Font") return AssetType::Font;
			else if (type == "FontFamily") return AssetType::FontFamily;
			else if (type == "Script") return AssetType::Script;
			else if (type == "Material") return AssetType::Material;
			else if (type == "Memory") return AssetType::MemoryAsset;

			return AssetType::None;
		}

		AssetType GetTypeFromExtension(std::string_view extension)
		{
			if (extension == ".nebula")			return AssetType::Scene;
			else if (extension == ".prefab")	return AssetType::Prefab;
			else if (extension == ".cs")		return AssetType::Script;
			else if (extension == ".mat")		return AssetType::Material;
			else if (extension == ".ttf" || extension == ".TTF")  return AssetType::Font;
			else if (extension == ".png" || extension == ".jpeg") return AssetType::Texture;

			return AssetType::None;
		}
	}

	// Note: Changing a global asset's index may mess up scenes and prefabs
	AssetRegistry AssetManagerBase::s_GlobalRegistry = {};
	uint16_t AssetManagerBase::s_GlobalIndex = 1;

	bool AssetManagerBase::IsHandleValid(AssetHandle handle)
	{
		return handle != 0 && (m_AssetRegistry.find(handle) != m_AssetRegistry.end() 
			|| s_GlobalRegistry.find(handle) != s_GlobalRegistry.end());
	}

	bool AssetManagerBase::IsAssetLoaded(AssetHandle handle)
	{
		return m_Assets.find(handle) != m_Assets.end();
	}

	AssetHandle AssetManagerBase::CreateAsset(const std::filesystem::path& path)
	{
		NB_PROFILE_FUNCTION();
		
		if (const AssetMetadata& data = GetAssetMetadata(path))
			return data.Handle;

		std::filesystem::path relativePath = std::filesystem::relative(path, Project::GetAssetDirectory());
		AssetHandle handle = AssetHandle();
		
		if (!CreateAsset(handle, path, relativePath))
			return NULL;

		return handle;
	}

	bool AssetManagerBase::CreateAsset(AssetHandle handle, const std::filesystem::path& path, const std::filesystem::path& relativePath)
	{
		NB_PROFILE_FUNCTION();
		
		if (IsHandleValid(handle)
			|| !std::filesystem::exists(path))
			return false;

		AssetType type = Utils::GetTypeFromExtension(path.extension().string());
		if (type == AssetType::None)
			return false;

		AssetMetadata data;
		data.Handle = handle;
		data.Type = type;
		data.Path = path;
		data.RelativePath = relativePath;
		data.Watcher = CreateRef<FileWatcher>(path, AssetImporter::OnAssetChange);

		m_AssetRegistry[handle] = data;
		return true;
	}
	
	bool AssetManagerBase::CreateAsset(AssetMetadata& metadata)
	{
		if (!metadata || IsHandleValid(metadata.Handle)
			|| !std::filesystem::exists(metadata.Path))
			return false;

		m_AssetRegistry[metadata.Handle] = metadata;
		return true;
	}

	AssetHandle AssetManagerBase::CreateMemoryAsset(Ref<Asset> asset)
	{
		NB_PROFILE_FUNCTION();

		AssetMetadata data;
		data.Handle = AssetHandle();
		data.Type = AssetType::MemoryAsset;
		
		m_AssetRegistry[data.Handle] = data;
		m_Assets[data.Handle] = asset;
		
		asset->Handle = data.Handle;
		return data.Handle;
	}

	bool AssetManagerBase::CreateGlobalAsset(AssetMetadata& metadata)
	{
		if (!metadata || !std::filesystem::exists(metadata.Path))
			return false;

		metadata.Handle = s_GlobalIndex;

		s_GlobalRegistry[s_GlobalIndex] = metadata;
		s_GlobalIndex++;
		return true;
	}
	
	AssetHandle AssetManagerBase::CreateGlobalAsset(const std::filesystem::path& path, const std::filesystem::path& relativePath)
	{
		NB_PROFILE_FUNCTION();

		if (!std::filesystem::exists(path))
			return NULL;

		AssetType type = Utils::GetTypeFromExtension(path.extension().string());
		if (type == AssetType::None)
			return NULL;

		AssetMetadata data;
		data.Handle = s_GlobalIndex;
		data.Type = type;
		data.Path = path;
		data.RelativePath = relativePath;
		data.isGlobal = true;

		s_GlobalRegistry[s_GlobalIndex] = data;
		s_GlobalIndex++;

		return data.Handle;
	}

	void AssetManagerBase::DeleteAsset(AssetHandle handle)
	{
		m_Assets.erase(handle);
		m_AssetRegistry.erase(handle);
	}

	AssetHandle AssetManagerBase::GetHandleFromPath(const std::filesystem::path& path)
	{
		NB_PROFILE_FUNCTION();

		for (const auto& [handle, asset] : m_AssetRegistry)
		{
			if (asset.Path == path || asset.RelativePath == path)
				return handle;
		}
		
		for (const auto& [handle, asset] : s_GlobalRegistry)
		{
			if (asset.Path == path || asset.RelativePath == path)
				return handle;
		}

		return NULL;
	}

	Ref<Asset> AssetManagerBase::FindAsset(AssetHandle handle)
	{
		auto it = m_Assets.find(handle);
		if (it != m_Assets.end())
			return it->second;

		return nullptr;
	}

	Ref<Asset> AssetManagerBase::GetAsset(AssetHandle handle, bool load)
	{
		NB_PROFILE_FUNCTION();

		if (!IsHandleValid(handle))
			return nullptr;

		if (IsAssetLoaded(handle))
			return FindAsset(handle);
		
		const AssetMetadata& metadata = GetAssetMetadata(handle);
		Ref<Asset> asset = AssetImporter::ImportAsset(handle, metadata);
		
		if (!asset)
			return nullptr;

		m_Assets[handle] = asset;
		return asset;
	}

	const AssetMetadata& AssetManagerBase::GetAssetMetadata(AssetHandle handle) const
	{
		static AssetMetadata s_NullMetadata;

		auto it = m_AssetRegistry.find(handle);
		if (it != m_AssetRegistry.end())
			return it->second;

		it = s_GlobalRegistry.find(handle);
		if (it != s_GlobalRegistry.end())
			return it->second;

		return s_NullMetadata;
	}

	const AssetMetadata& AssetManagerBase::GetAssetMetadata(const std::filesystem::path& path) const
	{
		NB_PROFILE_FUNCTION();
		static AssetMetadata s_NullMetadata;

		for (const auto& [handle, metadata] : m_AssetRegistry)
		{
			if (metadata.Path == path || metadata.RelativePath == path)
				return metadata;
		}
		
		for (const auto& [handle, metadata] : s_GlobalRegistry)
		{
			if (metadata.Path == path || metadata.RelativePath == path)
				return metadata;
		}

		return s_NullMetadata;
	}

	Array<AssetHandle> AssetManagerBase::GetAllAssetsWithType(AssetType type, bool global)
	{
		Array<AssetHandle> handles;
		GetAllAssetsWithType(handles, type, global);
		return handles;
	}

	void AssetManagerBase::GetAllAssetsWithType(Array<AssetHandle>& handlesArray, AssetType type, bool global)
	{
		NB_PROFILE_FUNCTION();

		for (const auto& [handle, metadata] : m_AssetRegistry)
		{
			if (metadata.Type != type)
				continue;

			handlesArray.push_back(handle);
		}
		
		if (!global)
			return;

		for (const auto& [handle, metadata] : s_GlobalRegistry)
		{
			if (metadata.Type != type)
				continue;

			handlesArray.push_back(handle);
		}
	}

	void AssetManagerBase::SerializeRegistry(const std::filesystem::path& path)
	{
		NB_PROFILE_FUNCTION();

		YAML::Emitter out; 
		out << YAML::BeginMap; // Root
		out << YAML::Key << "AssetRegistry" << YAML::Value;
		out << YAML::BeginSeq; // AssetRegistry

		for (const auto& [handle, metadata] : m_AssetRegistry)
		{
			if (metadata.Type == AssetType::MemoryAsset)
				continue;

			out << YAML::BeginMap;
			out << YAML::Key << "Handle" << handle;
			out << YAML::Key << "Type" << Utils::AssetTypeToString(metadata.Type);
			
			std::filesystem::path assetPath = std::filesystem::relative(metadata.Path, path.parent_path());
			out << YAML::Key << "Path" << assetPath.generic_string();
			out << YAML::Key << "RelativePath" << metadata.RelativePath.generic_string();
			out << YAML::EndMap;
		}

		out << YAML::EndSeq; // AssetRegistry
		out << YAML::EndMap; // Root

		std::ofstream fout(path);
		fout << out.c_str();
	}

	bool AssetManagerBase::DeserializeRegistry(const std::filesystem::path& path)
	{
		NB_PROFILE_FUNCTION();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(path.string());
		}
		catch (YAML::Exception e)
		{
			NB_ERROR("[AssetManager] Failed to load registry file '{0}'\n     {1}", path.string(), e.what());
			return false;
		}

		auto registryNode = data["AssetRegistry"];
		if (!registryNode)
			return false;

		for (const auto& node : registryNode)
		{
			AssetHandle handle = node["Handle"].as<AssetHandle>();
			std::string type = node["Type"].as<std::string>();
			std::filesystem::path assetPath = path.parent_path() / node["Path"].as<std::string>();
			
			if (!std::filesystem::exists(assetPath))
				continue;

			auto& metadata = m_AssetRegistry[handle];
			metadata.Handle = handle;
			metadata.Path = assetPath;
			metadata.RelativePath = node["RelativePath"].as<std::string>();
			metadata.Type = Utils::AssetTypeFromString(type);
			metadata.Watcher = CreateRef<FileWatcher>(assetPath, AssetImporter::OnAssetChange);
		}

		return true;
	}
}