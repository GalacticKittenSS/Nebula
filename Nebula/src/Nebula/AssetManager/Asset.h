#pragma once

#include "Nebula/Core/UUID.h"

#include <filesystem>
//#include "../filewatch/FileWatch.hpp"

namespace Nebula 
{
	using AssetHandle = UUID;

	enum class AssetType : uint16_t
	{
		None = 0,
		Scene,
		Prefab,
		Texture,
		Font,
		FontFamily,
		Script,
		MemoryAsset
	};

	struct AssetMetadata
	{
		AssetHandle Handle;
		AssetType Type;
		std::filesystem::path Path;
		std::filesystem::path RelativePath;

		operator bool() const { return Type != AssetType::None; }
	};

	class Asset
	{
	public:
		AssetHandle Handle = NULL;
		virtual AssetType GetType() const = 0;
	private:
		//Scope<filewatch::FileWatch<std::string>> Watcher;
		
		friend class AssetManagerBase;
	};

	namespace Utils
	{
		std::string AssetTypeToString(AssetType type);
		AssetType AssetTypeFromString(std::string_view type);
		AssetType GetTypeFromExtension(std::string_view extension);
	}
}