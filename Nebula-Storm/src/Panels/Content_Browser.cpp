#include "Content_Browser.h"

#include <Nebula/Scene/Prefab_Serializer.h>
#include <Nebula/AssetManager/TextureImporter.h>

#include <Nebula/Utils/UI.h>
#include <imgui_internal.h>

namespace Nebula {
	extern const std::filesystem::path s_AssetPath;
	
	ContentBrowserPanel::ContentBrowserPanel()
	{
		m_DirectoryIcon = TextureImporter::CreateTexture2D("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon = TextureImporter::CreateTexture2D("Resources/Icons/ContentBrowser/FileIcon.png");
		m_PrefabIcon = TextureImporter::CreateTexture2D("Resources/Icons/ContentBrowser/PrefabIcon.png");
	}

	void ContentBrowserPanel::SetContext(const std::filesystem::path& assetsPath)
	{
		m_BaseDirectory = assetsPath;
		m_CurrentDirectory = m_BaseDirectory;
	}

	void ContentBrowserPanel::SetSceneContext(const Ref<Scene>& scene)
	{
		m_Scene = scene;
	}

	void ContentBrowserPanel::OnImGuiRender() 
	{
		ImGui::Begin("Content Browser");
		RenderBrowser();
		CreateFilePopup();

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (m_Scene && !m_DragDrop && ImGui::BeginDragDropTargetCustom(window->ContentRegionRect, window->ID))
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
			{
				const UUID entityID = *(const UUID*)payload->Data;

				Entity entity = { entityID, m_Scene.get() };
				std::filesystem::path filepath = m_CurrentDirectory / (entity.GetName() + ".prefab");

				PrefabSerializer serializer(m_Scene.get());
				serializer.Serialize(entity, filepath.string());
			}
		}

		m_DragDrop = false;
		ImGui::End();
	}

	void ContentBrowserPanel::RenderBrowser()
	{
		static float size = 0.0f;

		if (m_BaseDirectory.string() == "")
			return;

		UI::ScopedStyleVar rounding(ImGuiStyleVar_FrameRounding, 7.5f);

		ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 200.0f);
		ImGui::SetNextItemWidth(200.0f);
		ImGui::SliderFloat("##thumbnail", &size, 0, 2);

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMin().x);

		if (m_CurrentDirectory != std::filesystem::path(m_BaseDirectory))
		{
			ImGuiIO& io = ImGui::GetIO();
			ImFont* boldFont = io.Fonts->Fonts[0];
			boldFont->Scale = size + 1;

			ImGui::PushFont(boldFont);
			float buttonSize = (size + 2) * 15.0f;
			if (ImGui::Button("<", ImVec2{ buttonSize, buttonSize }))
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			ImGui::PopFont();

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					std::filesystem::path filepath = (const wchar_t*)payload->Data;
					std::filesystem::rename(filepath, m_CurrentDirectory.parent_path() / filepath.filename());
				}
			}

			boldFont->Scale = 1;
		}

		float thumbnailSize = (size + 1) * 128.0f;
		float cellSize = thumbnailSize + 20.0f;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columCount = (int)(panelWidth / cellSize);
		if (columCount < 1)
			columCount = 1;

		ImGui::Columns(columCount, 0, false);

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory)) {
			const auto& path = directoryEntry.path();
			std::string filename = path.filename().string();

			ImGui::PushID(filename.c_str());

			Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;
			icon = path.extension() == ".prefab" ? m_PrefabIcon : icon;
			
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
			ImGui::PopStyleColor();

			if (ImGui::BeginDragDropSource()) {
				std::filesystem::path relativePath(path);
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}

			if (directoryEntry.is_directory())
			{
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					m_CurrentDirectory /= path.filename();

				if (ImGui::BeginDragDropTarget())
				{
					m_DragDrop = true;

					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						std::filesystem::path filepath = (const wchar_t*)payload->Data;
						std::filesystem::rename(filepath, directoryEntry / filepath.filename());
					}

					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
					{
						const UUID entityID = *(const UUID*)payload->Data;

						Entity entity = { entityID, m_Scene.get() };
						std::filesystem::path filepath = directoryEntry / (entity.GetName() + ".prefab");

						PrefabSerializer serializer(m_Scene.get());
						serializer.Serialize(entity, filepath.string());
					}
				}
			}

			UI::ScopedStyleVar rounding(ImGuiStyleVar_FrameRounding, 0.0f);
			UI::ScopedStyleColor colour(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));

			if (ImGui::BeginPopupContextItem())
			{
				if (!AssetManager::GetHandleFromPath(path) &&
					AssetManager::GetTypeFromExtension(path.extension().string()) != AssetType::None)
				{
					if (ImGui::Button("Import Asset"))
						AssetManager::CreateAsset(path);
				}

				if (directoryEntry.is_directory())
				{
					if (ImGui::Button("Delete Directory"))
						std::filesystem::remove_all(path.string().c_str());
				}
				else
				{
					if (ImGui::Button("Delete File"))
						std::filesystem::remove(path.string().c_str());
				}

				ImGui::EndPopup();
			}

			float text_width = ImGui::CalcTextSize(filename.c_str()).x;
			float text_indentation = (cellSize - text_width) * 0.5f;

			if (text_width > cellSize)
			{
				ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + cellSize - GImGui->Style.ItemSpacing.x);
				ImGui::TextWrapped(filename.c_str());
				ImGui::PopTextWrapPos();
			}
			else
			{
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + text_indentation - GImGui->Style.ItemSpacing.x);
				ImGui::Text(filename.c_str());
			}

			ImGui::NextColumn();

			ImGui::PopID();
		}

		ImGui::Columns(1);
	}

	void ContentBrowserPanel::CreateFilePopup()
	{
		bool open_popup = false;
		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::BeginMenu("Create New..."))
			{
				ImVec2 buttonSize = { 80.0f, 0.0f };

				if (ImGui::Button("Blank", buttonSize))
				{
					m_CreateFileName = "NewFile.txt";
					open_popup = true;
				}

				if (ImGui::Button("Directory", buttonSize))
				{
					m_CreateFileName = "NewDirectory";
					m_CreateDirectory = true;
					open_popup = true;
				}

				if (ImGui::Button("Scene", buttonSize))
				{
					m_CreateFileName = "NewScene.nebula";
					open_popup = true;
				}

				if (ImGui::Button("Script", buttonSize))
				{
					m_CreateFileName = "NewScript.cs";
					open_popup = true;
				}

				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		if (open_popup)
			ImGui::OpenPopup("Create File");

		if (ImGui::BeginPopup("Create File")) {
			ImGui::Text("Filename: ");
			ImGui::SameLine();

			char buffer[256];
			strncpy_s(buffer, sizeof(buffer), m_CreateFileName.c_str(), sizeof(buffer));

			if (ImGui::InputText("##Name", buffer, sizeof(buffer)))
				m_CreateFileName = std::string(buffer);

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());

			if (ImGui::Button("Create File"))
			{
				if (m_CreateDirectory)
				{
					std::filesystem::path directory = m_CurrentDirectory / std::filesystem::path(m_CreateFileName).replace_extension();
					std::filesystem::create_directory(directory);
				}
				else
				{
					std::string extension = m_CreateFileName.substr(m_CreateFileName.find("."));
					std::filesystem::path templateFile = "Resources/Templates/" + extension;
					std::filesystem::path newFile = m_CurrentDirectory / m_CreateFileName;

					if (std::filesystem::exists(templateFile))
						std::filesystem::copy(templateFile, newFile);
					else
						std::ofstream fstream(newFile);
				}

				m_CreateDirectory = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}
}