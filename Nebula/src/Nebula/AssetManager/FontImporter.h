#pragma once

#include "Nebula/Renderer/Fonts.h"

namespace Nebula
{
	class FontImporter
	{
	public:
		static Ref<Font> ImportFont(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<FontFamily> ImportFontFamily(AssetHandle handle, const AssetMetadata& metadata);
	};
}