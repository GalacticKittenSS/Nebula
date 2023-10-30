#pragma once

#include <Nebula.h>

#include "ThumbnailCache.h"

#include <filesystem>
#include <map>

namespace Nebula {
	class ContentBrowserPanel {
	public:
		ContentBrowserPanel();
		ContentBrowserPanel(Ref<Project> project);

		void OnImGuiRender();

		void RenderProperties();

		// For Creating Prefabs
		void SetContext(const Ref<Scene>& scene);
	private:
		void RenderBrowser();
		void RefreshAssetTree();
		void CreateFilePopup();
		Ref<Texture2D> GetIcon(const std::filesystem::path& extension);
	private:
		Ref<Project> m_Project;
		Ref<Scene> m_Scene;
		Ref<ThumbnailCache> m_ThumbnailCache;

		std::filesystem::path m_BaseDirectory;
		std::filesystem::path m_CurrentDirectory;
		std::filesystem::path m_SelectedFile;
		Ref<Texture2D> m_DirectoryIcon, m_FileIcon, m_PrefabIcon, m_FontIcon, m_MaterialIcon, m_ScriptIcon;

		AssetHandle m_AssetPreview;

		bool m_DragDrop = false;

		Array<std::filesystem::path> m_TreeNodes;
		bool m_OnlyShowAssets = false;

		std::string m_CreateFileName;
		bool m_CreateDirectory = false;
	};
}