#pragma once

#include <Nebula.h>

#include <filesystem>
#include <map>

namespace Nebula {
	class ContentBrowserPanel {
	public:
		ContentBrowserPanel();

		void OnImGuiRender();

		void RenderProperties();

		// For Creating Prefabs
		void SetSceneContext(const Ref<Scene>& scene);
		void SetContext(const std::filesystem::path& assetsPath);
	private:
		void RenderBrowser();
		void RefreshAssetTree();
		void CreateFilePopup();
	private:
		Ref<Scene> m_Scene;
		std::filesystem::path m_BaseDirectory;
		std::filesystem::path m_CurrentDirectory;
		std::filesystem::path m_SelectedFile;
		Ref<Texture2D> m_DirectoryIcon, m_FileIcon, m_PrefabIcon;

		Ref<Asset> m_AssetPreview;

		bool m_DragDrop = false;

		Array<std::filesystem::path> m_TreeNodes;
		bool m_OnlyShowAssets = false;

		std::string m_CreateFileName;
		bool m_CreateDirectory = false;
	};
}