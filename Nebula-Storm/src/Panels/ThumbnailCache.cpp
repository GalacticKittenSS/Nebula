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

		Ref<Texture2D> texture = TextureImporter::CreateTexture2D(absoulutePath.string());
		if (!texture)
			return nullptr;

		auto& cachedImage = m_CachedImages[assetPath];
		cachedImage.Timestamp = seconds;
		cachedImage.Image = texture;
		return cachedImage.Image;
	}
}