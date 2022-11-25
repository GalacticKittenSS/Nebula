#include "Content_Browser.h"

#include <imgui.h>

namespace Nebula {
	extern const std::filesystem::path s_AssetPath;
	
	ContentBrowserPanel::ContentBrowserPanel()
	{
		m_DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/FileIcon.png");
	}

	void ContentBrowserPanel::SetContext(const std::filesystem::path& assetsPath)
	{
		m_BaseDirectory = assetsPath;
		m_CurrentDirectory = m_BaseDirectory;
	}

	void ContentBrowserPanel::OnImGuiRender() 
	{
		ImGui::Begin("Content Browser");
		RenderBrowser();
		ImGui::End();
	}

	void ContentBrowserPanel::RenderBrowser()
	{
		static float size = 1.0f;
		ImGui::SliderFloat("##thumbnail", &size, 0, 2);

		float thumbnailSize = (size + 1) * 128.0f;
		float cellSize = thumbnailSize + 16.0f;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columCount = (int)(panelWidth / cellSize);
		if (columCount < 1)
			columCount = 1;

		if (m_BaseDirectory.string() == "")
			return;

		if (m_CurrentDirectory != std::filesystem::path(m_BaseDirectory)) {
			ImGuiIO& io = ImGui::GetIO();
			ImFont* boldFont = io.Fonts->Fonts[0];
			boldFont->Scale = size + 1;

			ImGui::PushFont(boldFont);
			float buttonSize = (size + 2) * 15.0f;
			if (ImGui::Button("<", ImVec2{ buttonSize, buttonSize }))
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			ImGui::PopFont();
			boldFont->Scale = 1;
		}

		ImGui::Columns(columCount, 0, false);

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory)) {
			const auto& path = directoryEntry.path();
			std::string filename = path.filename().string();

			ImGui::PushID(filename.c_str());

			Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
			ImGui::PopStyleColor();

			if (ImGui::BeginDragDropSource()) {
				std::filesystem::path relativePath(path);
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}


			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
				if (directoryEntry.is_directory())
					m_CurrentDirectory /= path.filename();
			}

			ImVec2 text = ImGui::CalcTextSize(filename.c_str());
			float text_width = text.x;

			float text_indentation = (cellSize - text_width) * 0.5f;
			float min_indentation = 20.0f;
			if (text_indentation <= min_indentation) {
				text_indentation = min_indentation;
			}

			ImGui::SameLine(text_indentation);
			ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + cellSize - text_indentation);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + cellSize);
			ImGui::TextWrapped(filename.c_str());
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - cellSize + text.y + ImGui::GetStyle().ItemSpacing.y * 3.0f);
			ImGui::PopTextWrapPos();

			ImGui::NextColumn();

			ImGui::PopID();
		}

		ImGui::Columns(1);
	}
}