#include "ThumbnailCache.h"

#include "Nebula/AssetManager/TextureImporter.h"

namespace Nebula
{
	ThumbnailCache::ThumbnailCache(Ref<Project> project)
		: m_Project(project)
	{
		m_ThumbnailCachePath = m_Project->GetAssetDirectory() / "Thumbnail.cache";
	}

	Ref<Texture2D> ThumbnailCache::GetorCreateThumbnail(const std::filesystem::path& assetPath)
	{
		auto absoulutePath = m_Project->GetAssetPath(assetPath);

		if (!std::filesystem::exists(absoulutePath))
			return nullptr;

		std::filesystem::file_time_type lastWriteTime = std::filesystem::last_write_time(absoulutePath);
		uint64_t seconds = std::chrono::duration_cast<std::chrono::seconds>(lastWriteTime.time_since_epoch()).count();

		if (m_CachedImages.find(assetPath) != m_CachedImages.end())
		{
			auto& cachedImage = m_CachedImages.at(assetPath);
			if (cachedImage.Timestamp == seconds)
				return cachedImage.Image;
		}
		
		if (AssetManager::GetTypeFromExtension(assetPath.extension().string()) != AssetType::Texture)
			return nullptr;

		ThumbnailInfo info;
		info.AbsolutePath = absoulutePath;
		info.AssetPath = assetPath;
		info.Timestamp = seconds;
		m_Queue.push(info);

		return nullptr;
	}

	void ThumbnailCache::Update()
	{
		while (!m_Queue.empty())
		{
			const auto& thumbnailInfo = m_Queue.front();

			if (m_CachedImages.find(thumbnailInfo.AssetPath) != m_CachedImages.end())
			{
				auto& cachedImage = m_CachedImages.at(thumbnailInfo.AssetPath);
				if (cachedImage.Timestamp == thumbnailInfo.Timestamp)
				{
					m_Queue.pop(); 
					continue;
				}
			}

			Ref<Texture2D> texture = TextureImporter::CreateTexture2D(thumbnailInfo.AbsolutePath.string());
			if (!texture)
			{
				m_Queue.pop(); 
				continue;
			}

			auto& cachedImage = m_CachedImages[thumbnailInfo.AssetPath];
			cachedImage.Timestamp = thumbnailInfo.Timestamp;
			cachedImage.Image = texture;
			m_Queue.pop();
			break;
		}
	}
}