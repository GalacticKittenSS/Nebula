#pragma once

#include <Nebula.h>
#include <map>
#include <queue>

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
		void Update();
	private:
		Ref<Project> m_Project;

		std::map<std::filesystem::path, ThumbnailImage> m_CachedImages;

		struct ThumbnailInfo
		{
			std::filesystem::path AbsolutePath;
			std::filesystem::path AssetPath;
			uint64_t Timestamp;
		};
		std::queue<ThumbnailInfo> m_Queue;

		std::filesystem::path m_ThumbnailCachePath;
	};
}