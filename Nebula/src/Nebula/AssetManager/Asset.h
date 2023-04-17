#pragma once

#include "AssetData.h"

#include <filesystem>
#include "../filewatch/FileWatch.hpp"

namespace Nebula 
{
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

	class Asset
	{
	public:
		~Asset() { delete Data; }

		AssetType Type;
		AssetHandle Handle;
		std::filesystem::path Path;
		std::filesystem::path RelativePath;

		bool IsLoaded = false;

		template <typename T>
		Ref<T> GetData();
		
		template <typename T>
		Ref<T> GetData(bool bold, bool italic);

		AssetData* Data = nullptr;
	private:
		Scope<filewatch::FileWatch<std::string>> Watcher;
		
		friend class AssetManagerBase;
	};
}