#pragma once

#include "Nebula/Core/UUID.h"
#include "Nebula/Renderer/Texture.h"
#include "Nebula/Renderer/Fonts.h"

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

	struct FontAsset : AssetTypeData
	{
		FontAsset() = default;

		FontAsset(const std::filesystem::path& path)
			: FontAsset(path.string(), path)
		{
		}

		FontAsset(const std::string& name, const std::filesystem::path& path)
		{
			Data = CreateRef<Font>(name, path);
			FontManager::Add(Data);

			IsLoaded = Data->GetAtlasTexture() != nullptr;
		}

		Ref<Font> Data;
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
		const T& GetData() { return *(T*)Data; }
	private:
		Scope<filewatch::FileWatch<std::string>> Watcher;
		AssetTypeData* Data;
		
		friend class AssetManager;
	};
}