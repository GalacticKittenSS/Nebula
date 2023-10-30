#pragma once

#include <Nebula.h>
#include <map>

namespace Nebula
{
	struct ThumbnailImage
	{
		uint64_t Timestamp;
		Ref<Texture2D> Image;
	};

	class ThumbnailCache
	{
	public:
		ThumbnailCache(Ref<Project> projcet);

		Ref<Texture2D> GetorCreateThumbnail(const std::filesystem::path& path);
	private:
		Ref<Project> m_Project;

		std::map<std::filesystem::path, ThumbnailImage> m_CachedImages;

		std::filesystem::path m_ThumbnailCachePath;
	};
}