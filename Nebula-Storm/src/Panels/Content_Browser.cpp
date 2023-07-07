#include "Content_Browser.h"

#include <Nebula/Scene/Prefab_Serializer.h>
#include <Nebula/AssetManager/TextureImporter.h>

#include <Nebula/Maths/MinMax.h>
#include <Nebula/Utils/UI.h>
#include <imgui_internal.h>

namespace Nebula {
	static float s_TextColumnWidth = 105.0f;
	static float s_MaxItemWidth = 425.0f;
	
	ContentBrowserPanel::ContentBrowserPanel()
	{
		m_DirectoryIcon = TextureImporter::CreateTexture2D("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon = TextureImporter::CreateTexture2D("Resources/Icons/ContentBrowser/FileIcon.png");
		m_PrefabIcon = TextureImporter::CreateTexture2D("Resources/Icons/ContentBrowser/PrefabIcon.png");
		m_FontIcon = TextureImporter::CreateTexture2D("Resources/Icons/ContentBrowser/FontIcon.png");
		m_MaterialIcon = TextureImporter::CreateTexture2D("Resources/Icons/ContentBrowser/MaterialIcon.png");
		m_ScriptIcon = TextureImporter::CreateTexture2D("Resources/Icons/ContentBrowser/ScriptIcon.png");
	}

	void ContentBrowserPanel::SetContext(const std::filesystem::path& assetsPath)
	{
		m_BaseDirectory = assetsPath;
		m_CurrentDirectory = m_BaseDirectory;
		RefreshAssetTree();
	}

	void ContentBrowserPanel::SetSceneContext(const Ref<Scene>& scene)
	{
		m_Scene = scene;
	}

	static float DrawLabel(std::string label)
	{
		float padding = ImGui::GetStyle().ItemSpacing.x * 2;
		float max_width = Maths::Max(s_TextColumnWidth, ImGui::GetWindowContentRegionMax().x - s_MaxItemWidth);

		if (ImGui::CalcTextSize(label.c_str()).x + padding > max_width)
		{
			int i = 0;
			for (; i < label.length(); i++)
			{
				if (ImGui::CalcTextSize(label.c_str(), &label[i]).x + padding > max_width)
					break;
			}

			label = label.substr(0, i - 3) + "...";
		}

		ImGui::Text(label.c_str());
		ImGui::SameLine();

		float size = Maths::Min(s_MaxItemWidth, ImGui::GetWindowContentRegionMax().x - max_width - padding);
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - size);
		return size;
	}

	static bool DrawVec1Control(std::string label, float& values, const float& min = 0.0f, const float& max = 0.0f,
		const float& resetvalue = 0.0f, float step = 0.1f)
	{
		ImGui::PushID(label.c_str());
		float size = DrawLabel(label);
		ImGui::SetNextItemWidth(size);

		bool open = ImGui::DragFloat("##V", &values, step, min, max, "%.2f");
		ImGui::PopID();
		return open;
	}

	static bool DrawColourEdit(const std::string& label, glm::vec4& colour)
	{
		ImGui::PushID(label.c_str());
		float size = DrawLabel(label);
		const ImVec4 col_v4(colour.x, colour.y, colour.z, colour.w);

		if (ImGui::ColorButton("##button", col_v4, 0, ImVec2{ size, 0.0f }))
			ImGui::OpenPopup("picker");

		bool changed = false;
		if (ImGui::BeginPopup("picker"))
		{
			changed = ImGui::ColorPicker4("##picker", glm::value_ptr(colour));
			ImGui::EndPopup();
		}

		ImGui::PopID();
		return changed;
	}

	void ContentBrowserPanel::OnImGuiRender() 
	{
		NB_PROFILE_FUNCTION();

		ImGui::Begin("Content Browser");
		RenderBrowser();

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

				AssetManager::CreateAsset(filepath);
				RefreshAssetTree();
			}
		}

		m_DragDrop = false;
		ImGui::End();

		ImGui::Begin("File Properties");
		RenderProperties();
		ImGui::End();
	}

	Ref<Texture2D> ContentBrowserPanel::GetIcon(const std::filesystem::path& path)
	{
		AssetType type = AssetManager::GetTypeFromExtension(path.extension().string());
		switch (type)
		{
		case AssetType::Prefab: return m_PrefabIcon;
		case AssetType::Font: return m_FontIcon;
		case AssetType::Script: return m_ScriptIcon;
		case AssetType::Material: return m_MaterialIcon;
		case AssetType::Texture:
		{
			AssetHandle handle = AssetManager::GetHandleFromPath(path);
			return AssetManager::GetAsset<Texture2D>(handle);
		}
		}
		
		return m_FileIcon;
	}

	void ContentBrowserPanel::RenderBrowser()
	{
		static float size = 0.0f;

		if (m_BaseDirectory.string() == "")
			return;

		UI::ScopedStyleVar rounding(ImGuiStyleVar_FrameRounding, 7.5f);

		ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 240.0f);
		ImGui::SetNextItemWidth(200.0f);
		ImGui::SliderFloat("##thumbnail", &size, 0, 2);

		ImGui::SameLine();

		if (ImGui::ImageButton((ImTextureID)m_DirectoryIcon->GetRendererID(), ImVec2{ 20.0f, 20.0f }, { 0, 1 }, { 1, 0 }))
			m_OnlyShowAssets = !m_OnlyShowAssets;
		
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltipEx(ImGuiWindowFlags_NoBackground, ImGuiTooltipFlags_None);
			GImGui->FontSize /= 1.1f;
			ImGui::Text("Toggle Imported Assets");
			GImGui->FontSize *= 1.1f;
			ImGui::EndTooltip();
		}

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
		
		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			std::filesystem::path path = std::filesystem::relative(directoryEntry, Project::GetAssetDirectory());
			std::string filename = path.filename().string();

			bool isAsset = m_TreeNodes.find(path) < m_TreeNodes.size();
			if (m_OnlyShowAssets && !isAsset)
				continue;

			ImGui::PushID(filename.c_str());
			
			Ref<Texture2D> icon = m_DirectoryIcon;
			if (!directoryEntry.is_directory())
				icon = GetIcon(path);

			ImVec4 tint = ImVec4(1.0f, 1.0f, 1.0f, isAsset ? 1.0f : 0.5f);
			if (AssetManager::GetTypeFromExtension(path.extension().string()) == AssetType::Material)
			{
				AssetHandle handle = AssetManager::GetHandleFromPath(path);
				Ref<Material> mat = AssetManager::GetAsset<Material>(handle);
				if (mat)
					tint = ImVec4(mat->Colour.r, mat->Colour.g, mat->Colour.b, mat->Colour.a);
			}

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			
			if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 }, -1, ImVec4(), tint))
			{
				if (!directoryEntry.is_directory())
				{
					m_SelectedFile = directoryEntry;
					m_AssetPreview = AssetManager::GetHandleFromPath(m_SelectedFile);
				}
			}

			ImGui::PopStyleColor();

			if (ImGui::BeginDragDropSource())
			{
				std::filesystem::path relativePath(path);
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}

			if (directoryEntry.is_directory())
			{
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					m_CurrentDirectory /= filename;

				if (ImGui::BeginDragDropTarget())
				{
					m_DragDrop = true;

					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						std::filesystem::path filepath = (const wchar_t*)payload->Data;
						std::filesystem::rename(Project::GetAssetDirectory() / filepath, directoryEntry / filepath.filename());
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
				if (AssetManager::GetTypeFromExtension(path.extension().string()) != AssetType::None)
				{
					if (!isAsset)
					{
						if (ImGui::Button("Import Asset"))
						{
							AssetManager::CreateAsset(directoryEntry);
							RefreshAssetTree();
						}
					}
					else
					{
						if (ImGui::Button("Remove Asset"))
						{
							AssetHandle handle = AssetManager::GetHandleFromPath(directoryEntry);
							AssetManager::DeleteAsset(handle);
							RefreshAssetTree();
						}
					}
				}

				if (ImGui::Button("Delete"))
					ImGui::OpenPopup("DeleteFile");

				if (ImGui::BeginPopup("DeleteFile"))
				{
					ImGui::Text("Are you sure you want to delete?");
					
					if (ImGui::Button("Delete"))
					{
						AssetHandle handle = AssetManager::GetHandleFromPath(directoryEntry);
						AssetManager::DeleteAsset(handle);
						RefreshAssetTree();

						if (directoryEntry.is_directory())
							std::filesystem::remove_all(directoryEntry);
						else
							std::filesystem::remove(directoryEntry);
					}

					ImGui::SameLine();

					if (ImGui::Button("Cancel"))
						ImGui::CloseCurrentPopup();
					
					ImGui::EndPopup();
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

		CreateFilePopup();

		ImGui::Columns(1);
	}

	static void DrawFile(const std::filesystem::path& path)
	{
		std::ifstream file(path);
		if (file.is_open())
		{
			std::string line;
			while (std::getline(file, line))
			{
				ImGui::Text(line.c_str());
			}

			file.close();
		}
	}

	void ContentBrowserPanel::RenderProperties()
	{
		if (!std::filesystem::exists(m_SelectedFile))
			return;

		ImGuiIO& io = ImGui::GetIO();
		ImFont* boldFont = io.Fonts->Fonts[0];
		
		std::filesystem::path relative = std::filesystem::relative(m_SelectedFile, m_BaseDirectory);

		ImGui::PushFont(boldFont);
		GImGui->FontSize *= 1.5f;
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x - ImGui::CalcTextSize(relative.string().c_str()).x) / 2);
		ImGui::Text(relative.string().c_str());
		ImGui::PopFont();

		ImGui::Separator();
		ImGui::Separator();

		if (!m_AssetPreview)
		{
			DrawFile(m_SelectedFile);
			return;
		}

		AssetType type = AssetManager::GetTypeFromExtension(m_SelectedFile.extension().string());
		switch (type)
		{
		case AssetType::Material:
		{
			Ref<Material> material = AssetManager::GetAsset<Material>(m_AssetPreview);

			bool colour = DrawColourEdit("Colour", material->Colour);
			bool tiling = DrawVec1Control("Tiling", material->Tiling);

			DrawLabel("Texture");
			std::string text = "None";
			if (material->Texture)
			{
				auto& metadata = AssetManager::GetAssetMetadata(material->Texture->Handle);
				if (metadata)
					text = metadata.RelativePath.string();
			}

			bool texture = false;
			if (ImGui::Button(text.c_str(), ImVec2{ ImGui::GetContentRegionAvailWidth(), 0 }))
			{
				material->Texture = nullptr;
				texture = true;
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* payloadPath = (const wchar_t*)payload->Data;
					std::filesystem::path path = payloadPath;

					if (AssetManager::GetTypeFromExtension(path.extension().string()) == AssetType::Texture)
					{
						AssetHandle handle = AssetManager::CreateAsset(path);
						material->Texture = AssetManager::GetAsset<Texture2D>(handle);
						texture = true;
					}
				}
			}

			if (colour || tiling || texture)
			{
				MaterialSerializer serializer(material);
				serializer.Serialize(m_SelectedFile.string());
			}
			break;
		}
		case AssetType::Texture:
		{
			Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(m_AssetPreview);
			float buttonSize = ImGui::GetContentRegionAvailWidth();
			ImGui::Image((ImTextureID)texture->GetRendererID(), ImVec2{ buttonSize, buttonSize }, { 0, 1 }, { 1, 0 });
			break;
		}
		case AssetType::Font:
		{
			Ref<Font> font = AssetManager::GetAsset<Font>(m_AssetPreview);
			Ref<Texture2D> atlasTexture = font->GetAtlasTexture();

			if (atlasTexture)
			{
				float buttonSize = ImGui::GetContentRegionAvailWidth();
				ImGui::Image((ImTextureID)atlasTexture->GetRendererID(), ImVec2{ buttonSize, buttonSize }, { 0, 1 }, { 1, 0 });
			}

			break;
		}
		default:
		{
			DrawFile(m_SelectedFile);
			break;
		}
		}
	}

	void ContentBrowserPanel::RefreshAssetTree()
	{
		m_TreeNodes.clear();

		const auto& assetRegistry = Project::GetActive()->GetAssetManager()->GetAssetRegistry();
		for (const auto& [handle, metadata] : assetRegistry)
		{
			std::filesystem::path currentPath = "";
			
			for (const auto& p : metadata.RelativePath)
			{
				currentPath /= p;

				uint32_t index = m_TreeNodes.find(currentPath);
				if (index >= m_TreeNodes.size())
					m_TreeNodes.push_back(currentPath);
			}
		}
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