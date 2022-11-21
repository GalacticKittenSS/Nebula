#pragma once

#include <Nebula.h>
#include <filesystem>

namespace Nebula {
	class ContentBrowserPanel {
	public:
		ContentBrowserPanel();

		void OnImGuiRender();

		void SetContext(const std::filesystem::path& assetsPath);
	private:
		void RenderBrowser();
	private:
		std::filesystem::path m_BaseDirectory;
		std::filesystem::path m_CurrentDirectory;
		Ref<Texture2D> m_DirectoryIcon, m_FileIcon;
	};
}