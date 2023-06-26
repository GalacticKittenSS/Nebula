#pragma once

#include <Nebula.h>

#include <filesystem>
#include <map>

namespace Nebula {
	class ContentBrowserPanel {
	public:
		ContentBrowserPanel();

		void OnImGuiRender();

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
		Ref<Texture2D> m_DirectoryIcon, m_FileIcon, m_PrefabIcon;

		bool m_DragDrop = false;

		Array<std::filesystem::path> m_TreeNodes;
		bool m_OnlyShowAssets = false;

		std::string m_CreateFileName;
		bool m_CreateDirectory = false;
	};
}