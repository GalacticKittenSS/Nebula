#include "nbpch.h"
#include "FontImporter.h"

#include "Nebula/Project/Project.h"
#include "Nebula/Core/Buffer.h"

#include <iostream>
#include <fstream>

namespace Nebula
{
	Ref<Font> FontImporter::ImportFont(AssetHandle handle, const AssetMetadata& metadata)
	{
		std::filesystem::path path = metadata.Path;
		
		std::filesystem::path filename = metadata.RelativePath;
		filename = filename.replace_extension();
		
		std::filesystem::path cachePath =
			Project::GetActive()->GetProjectDirectory() /
			"Cache" / 
			(filename.string() + ".png");

		return CreateRef<Font>(path, cachePath);
	}
	
	Ref<FontFamily> FontImporter::ImportFontFamily(AssetHandle handle, const AssetMetadata& metadata)
	{
		NB_PROFILE_FUNCTION();

		std::filesystem::path path = metadata.Path;
		std::string name = metadata.RelativePath.string();

		struct FontData {
			AssetHandle Handle;
			std::string Name;
		};

		FontData fonts[4] = {
			{ NULL, "Regular" },
			{ NULL, "Bold" },
			{ NULL, "BoldItalic" },
			{ NULL, "Italic" }
		};

		for (uint32_t i = 0; i < 4; i++)
		{
			std::filesystem::path fontPath = path / (fonts[i].Name + ".ttf");

			if (const AssetMetadata& data = Project::GetAssetManager()->GetAssetMetadata(fontPath))
			{
				fonts[i].Handle = data.Handle;
				continue;
			}

			std::filesystem::path relativePath = name + "_" + fonts[i].Name;
			AssetHandle handle = AssetHandle();

			if (!Project::GetAssetManager()->CreateAsset(handle, fontPath, relativePath))
				continue;

			fonts[i].Handle = handle;
		}
		
		Ref<FontFamily> family = CreateRef<FontFamily>();
		family->Regular =	 fonts[0].Handle;
		family->Bold =		 fonts[1].Handle;
		family->BoldItalic = fonts[2].Handle;
		family->Italic =	 fonts[3].Handle;
		return family;
	}
}