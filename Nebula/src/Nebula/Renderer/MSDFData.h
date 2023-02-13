#pragma once

#include <vector>

#undef INFINITE
#include "FontGeometry.h"
#include "GlyphGeometry.h"

namespace Nebula
{
	struct MSDFData
	{
		std::vector<msdf_atlas::GlyphGeometry> Glyphs;
		msdf_atlas::FontGeometry FontGeometry;
	};
}