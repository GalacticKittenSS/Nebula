#pragma once

#include "Nebula/Renderer/Texture.h"

namespace Nebula
{
	class TextureImporter
	{
	public:
		static Ref<Texture2D> ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<Texture2D> CreateTexture2D(std::string_view path, bool imgui_usable = true);
	};
}