#include "nbpch.h"
#include "TextureImporter.h"

#include <stb_image.h>

namespace Nebula
{
	Ref<Texture2D> TextureImporter::ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata)
	{
		return CreateTexture2D(metadata.Path.string());
	}

	Ref<Texture2D> TextureImporter::CreateTexture2D(std::string_view path, bool imgui_usable)
	{
		NB_PROFILE_FUNCTION();

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		
		Buffer data;
		data.Data = stbi_load(path.data(), &width, &height, &channels, 0);
		data.Size = width * height * channels;

		if (!data)
		{
			NB_ERROR("[Texture Importer] Failed loading texture from path: {}", path);
			return nullptr;
		}

		TextureSpecification spec;
		spec.Width = width;
		spec.Height = height;
		spec.GenerateMips = true;
		spec.ImGuiUsable = imgui_usable;

		switch (channels)
		{
		case 3:
			spec.Format = ImageFormat::RGB8;
			break;
		case 4:
			spec.Format = ImageFormat::RGBA8;
			break;
		}

		Ref<Texture2D> texture = Texture2D::Create(spec, data);
		data.Release();
		return texture;
	}
}