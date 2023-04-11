#pragma once

#include "AssetData.h"

#include "Nebula/Core/UUID.h"

#include <filesystem>
#include "../filewatch/FileWatch.hpp"

namespace Nebula 
{
	typedef UUID AssetHandle;
	
	enum class AssetType : uint16_t
	{
		None = 0,
		Scene,
		Prefab,
		Texture,
		Font,
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
	private:
		Scope<filewatch::FileWatch<std::string>> Watcher;
		AssetData* Data = nullptr;
		
		friend class AssetManagerBase;
	};
}