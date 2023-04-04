#pragma once

#include "Nebula/Core/UUID.h"
#include "Nebula/Renderer/Texture.h"

#include "../filewatch/FileWatch.hpp"

#include <filesystem>

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

	struct AssetTypeData 
	{
		bool IsLoaded = false;
	};

	struct TextureAsset : AssetTypeData
	{
		TextureAsset() = default;
			
		TextureAsset(const std::filesystem::path& path)
		{
			Texture = Texture2D::Create(path.string());
			IsLoaded = Texture->IsLoaded();
		}

		Ref<Texture2D> Texture;
	};

	class Asset
	{
	public:
		~Asset() { delete Data; }

		AssetType Type;
		AssetHandle Handle;
		std::filesystem::path Path;

		bool IsLoaded = false;

		template <typename T>
		const T& GetData() { return *(T*)Data; }
	private:
		Scope<filewatch::FileWatch<std::string>> Watcher;
		AssetTypeData* Data;
		
		friend class AssetManager;
	};
}