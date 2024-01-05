#include "Scene_Hierarchy.h"

#include "Nebula/Maths/MinMax.h"
#include "Nebula/Utils/UI.h"

#include <Nebula/Scene/Prefab_Serializer.h>
#include <imgui_internal.h>

#include <cstring>
#include <filesystem>

#include "../Modules/imgui/misc/cpp/imgui_stdlib.h"
#include "../../Modules/Box2D/include/box2d/b2_body.h"
#include "../../Modules/Box2D/include/box2d/b2_fixture.h"

namespace Nebula {
	static float s_TextColumnWidth	= 100.0f;
	static float s_MaxItemWidth		= 425.0f;

	struct RectData {
		UUID Parent;
		ImRect Rect;
		uint32_t indexAbove;
		uint32_t indexBelow;
	};

	static bool DroppedIntoChildren(Ref<Scene> scene, UUID child, UUID parentEntity) {
		if (parentEntity == child)
			return true;

		UUID parent = scene->GetEntityNode(parentEntity).Parent;
		if (!parent)
			return false;

		return DroppedIntoChildren(scene, child, parent);
	}

	static void AddParent(Ref<Scene> scene, UUID childID, Entity parentEntity) {
		UUID parentID = parentEntity.GetUUID();
		auto& parent = scene->GetEntityNode(parentID);
		auto& child = scene->GetEntityNode(childID);

		//Check if Dropped Entity is being dropped to parent of itself
		if (child.Parent == parentID)
			return;

		if (DroppedIntoChildren(scene, childID, parentID))
			return;

		//Go Ahead if safe
		UUID pid = child.Parent;
		if (pid) {
			auto& pcc = scene->GetEntityNode(pid);
			pcc.Children.remove(childID);
		}

		parent.Children.push_back(childID);
		scene->m_SceneOrder.remove(childID);
		
		child.Parent = parentID;
		parentEntity.UpdateTransform();
	}
	
	static void EntityPayload(Scene* currentScene) {
		const ImGuiPayload* payload = ImGui::GetDragDropPayload();
		if (!payload)
			return;

		UUID uuid = *(const UUID*)payload->Data;
		auto& ParentComp = currentScene->GetEntityNode(uuid);

		if (!ParentComp.Parent)
			return;

		currentScene->GetEntityNode(ParentComp.Parent).Children.remove(uuid);
		
		UUID parentsParent = currentScene->GetEntityNode(ParentComp.Parent).Parent;
		ParentComp.Parent = parentsParent;

		if (parentsParent)
			currentScene->GetEntityNode(parentsParent).Children.push_back(uuid);
	}

	static bool DrawVec2Control(const std::string& label, glm::vec2& values, const glm::vec2& min = glm::vec2(0.0f), const glm::vec2& max = glm::vec2(0.0f),
		const glm::vec2& resetvalue = glm::vec2(0.0f));
	static bool DrawBool(const std::string& label, bool& values);
	static bool DrawColourEdit(const std::string& label, glm::vec4& colour);

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene) {
		SetContext(scene);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context) {
		if (m_SelectionContext)
			m_SelectionContext = { m_SelectionContext.GetUUID(), context.get() };
		
		m_Context = context;
	}

	void SceneHierarchyPanel::OnImGuiRender() {
		NB_PROFILE_FUNCTION();

		Rects.clear();

		ImGui::Begin("Scene Hierarchy");

		DrawSceneHierarchy();

		m_HierarchyFocused = ImGui::IsWindowFocused();
		m_HierarchyHovered = ImGui::IsWindowHovered();
		
		const ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (ImGui::BeginDragDropTargetCustom(window->ContentRegionRect, window->ID))
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				std::filesystem::path filepath = (const wchar_t*)payload->Data;
				std::filesystem::path assetPath = Project::GetAssetPath(filepath);

				PrefabSerializer serializer(m_Context.get());
				serializer.Deserialize(assetPath.string());
			}

			ImGui::EndDragDropTarget();
		}
		
		ImGui::End();


		ImGui::Begin("Project Settings");

		ProjectConfig& pConfig = Project::GetActive()->GetConfig();
		DrawVec2Control("Gravity", pConfig.Gravity, glm::vec2(0.0f), glm::vec2(0.0f), glm::vec2(0.0f, -9.81f));

		DrawColourEdit("Background Colour", pConfig.ClearColour);
		DrawBool("Show Sky", pConfig.ShowSky);

		ImGui::Separator();
		
		const ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed;
		
		if (ImGui::TreeNodeEx("Labels", treeFlags))
		{
			for (auto& [index, layer] : pConfig.Layers)
			{
				ImGui::PushID(index);

				char buffer[256];
				strncpy_s(buffer, sizeof(buffer), layer->Name.c_str(), sizeof(buffer));

				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

				if (ImGui::InputText("##Name", buffer, sizeof(buffer)))
					layer->Name = std::string(buffer);

				ImGui::PopID();
			}

			ImGui::TreePop();
		}

		ImGui::End();

		ImGui::Begin("Properties");

		if (m_SelectionContext)
			DrawComponents(m_SelectionContext);

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawSceneHierarchy() {
		if (!m_Context)
			return;
		
		DrawArray(m_Context->m_SceneOrder);

		for (uint32_t i = 0; i < Rects.size(); i++) {
			RectData* data = Rects[i];
			if (!ImGui::IsMouseHoveringRect(data->Rect.Min, data->Rect.Max)) continue;

			if (ImGui::BeginDragDropTargetCustom(data->Rect, 1)) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY")) {
					const UUID entityID = *(const UUID*)payload->Data;
					
					if (data->Parent) {
						Entity parent = { data->Parent, m_Context.get() };
						AddParent(m_Context, entityID, parent);
						
						Array<UUID>& children = m_Context->GetEntityNode(data->Parent).Children;

						size_t entityIndex = children.find(entityID);
						size_t newIndex = entityIndex;

						if (entityIndex > data->indexBelow)
							newIndex = data->indexBelow;

						if (entityIndex < data->indexAbove)
							newIndex = data->indexAbove;
						
						children.move(entityIndex, newIndex);
					} 
					else 
					{
						Entity entity = { entityID, m_Context.get() };
						
						if (UUID parentID = m_Context->GetEntityNode(entityID).Parent)
						{
							m_Context->GetEntityNode(parentID).Children.remove(entityID);
							m_Context->GetEntityNode(entityID).Parent = NULL;
							m_Context->m_SceneOrder.push_back(entityID);

							entity.UpdateTransform();
						}
						
						size_t entityIndex = m_Context->m_SceneOrder.find(entityID);
						size_t newIndex = entityIndex;

						if (entityIndex > data->indexBelow)
							newIndex = data->indexBelow;

						if (entityIndex < data->indexAbove)
							newIndex = data->indexAbove;

						m_Context->m_SceneOrder.move(entityIndex, newIndex);
					}
				}
				ImGui::EndDragDropTarget();
			}

			delete data;
			break;
		}

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectionContext = {};

		if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverExistingPopup)) {
			DisplayCreateEntity();
			ImGui::EndPopup();
		}

		if (ImGui::IsMouseReleased(0) && ImGui::IsWindowHovered() && !ImGui::IsDragDropPayloadBeingAccepted())
			EntityPayload(m_Context.get());
	}

	void SceneHierarchyPanel::DisplayCreateEntity(Entity parent)
	{
		if (ImGui::BeginMenu("Create Entity")) 
		{
			if (ImGui::MenuItem("Empty"))
			{
				auto& entity = m_Context->CreateEntity("Entity");

				if (parent)
					AddParent(m_Context, entity.GetUUID(), parent);
			}

			if (ImGui::MenuItem("Sprite")) 
			{
				auto& sprite = m_Context->CreateEntity("Sprite");
				sprite.AddComponent<SpriteRendererComponent>();
				sprite.AddComponent<Rigidbody2DComponent>();
				sprite.AddComponent<BoxCollider2DComponent>();
				
				if (parent)
					AddParent(m_Context, sprite.GetUUID(), parent);
			}

			if (ImGui::MenuItem("Circle")) 
			{
				auto& sprite = m_Context->CreateEntity("Circle");
				sprite.AddComponent<CircleRendererComponent>();
				sprite.AddComponent<Rigidbody2DComponent>();
				sprite.AddComponent<CircleColliderComponent>();

				if (parent)
					AddParent(m_Context, sprite.GetUUID(), parent);
			}

			if (ImGui::MenuItem("Camera")) 
			{
				auto& cam = m_Context->CreateEntity("Camera");
				cam.AddComponent<CameraComponent>();

				if (parent)
					AddParent(m_Context, cam.GetUUID(), parent);
			}

			ImGui::EndMenu();
		}
	}

	void SceneHierarchyPanel::DrawArray(Array<UUID>& entities) {
		if (entities.size()) {
			ImVec2 cursorPos = ImGui::GetCursorPos();
			ImVec2 elementSize = ImGui::GetItemRectSize();
			elementSize.x -= ImGui::GetStyle().FramePadding.x;
			elementSize.y = ImGui::GetStyle().FramePadding.y;
			cursorPos.y -= ImGui::GetStyle().FramePadding.y;
			ImVec2 windowPos = ImGui::GetCurrentWindow()->Pos;
			RectData* data = new RectData();
			data->Parent = m_Context->GetEntityNode(entities[0]).Parent;
			data->Rect = ImRect(windowPos.x + cursorPos.x, windowPos.y + cursorPos.y, windowPos.x + cursorPos.x + elementSize.x, windowPos.y + cursorPos.y + elementSize.y);
			data->indexAbove = 0;
			data->indexBelow = 0;
			Rects.push_back(data);
		}
		
		for (uint32_t n = 0; n < entities.size(); n++) {
			Entity entity{ entities[n], m_Context.get() };
			DrawEntityNode(entity, n);
		}
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity, uint32_t index) {
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		UUID uuid = entity.GetUUID();
		auto& pcc = m_Context->m_Nodes.at(uuid);

		ImGuiTreeNodeFlags flags = m_SelectionContext == entity ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding;

		if (pcc.Children.size() == 0)
			flags |= ImGuiTreeNodeFlags_Leaf;

		bool opened = ImGui::TreeNodeEx((void*)(uint32_t)entity, flags, tag.c_str());

		if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(0))
			m_SelectionContext = entity;

		if (ImGui::BeginDragDropSource()) {
			UUID entityID = entity.GetUUID();
			ImGui::SetDragDropPayload("ENTITY", &entityID, sizeof(uint64_t), ImGuiCond_Once);
			ImGui::Text(tag.c_str());
			ImGui::EndDragDropSource();
		}
		
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
				AddParent(m_Context, *(const UUID*)payload->Data, entity);
			
			ImGui::EndDragDropTarget();
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem()) {
			DisplayCreateEntity(entity);

			if (ImGui::MenuItem("Duplicate Entity"))
				m_Context->DuplicateEntity(entity);

			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (opened) {
			DrawArray(pcc.Children);
			ImGui::TreePop();
		}
		
		if (entityDeleted) {
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
			return;
		}

		ImVec2 cursorPos = ImGui::GetCursorPos();
		ImVec2 elementSize = ImGui::GetItemRectSize();
		elementSize.x -= ImGui::GetStyle().FramePadding.x;
		elementSize.y = ImGui::GetStyle().FramePadding.y;
		cursorPos.y -= ImGui::GetStyle().FramePadding.y;
		ImVec2 windowPos = ImGui::GetCurrentWindow()->Pos;
		RectData* data = new RectData();
		data->Parent = pcc.Parent;

		if (index == 0)
			elementSize.x = ImGui::GetContentRegionAvail().x;

		data->Rect = ImRect(windowPos.x + cursorPos.x, windowPos.y + cursorPos.y, windowPos.x + cursorPos.x + elementSize.x, windowPos.y + cursorPos.y + elementSize.y);
		data->indexAbove = index;
		data->indexBelow = index + 1;

		Rects.push_back(data);
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

	static bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetvalue = 0.0f) 
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());
		float size = DrawLabel(label);
		
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
		float item_width = (size - buttonSize.x * 3 - GImGui->Style.ItemSpacing.x * 2) / 3;

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, GImGui->Style.ItemSpacing.y });
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.2f, 0.1f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 1.0f, 0.3f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.2f, 0.1f, 1.0f });
		ImGui::PushFont(boldFont);

		bool xb = ImGui::Button("X", buttonSize);
		if (xb)
			values.x = resetvalue;
		
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();

		ImGui::SetNextItemWidth(item_width);
		bool x = ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopStyleVar();

		ImGui::SameLine();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, GImGui->Style.ItemSpacing.y });
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);

		bool yb = ImGui::Button("Y", buttonSize);
		if (yb)
			values.y = resetvalue;
		
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		
		ImGui::SetNextItemWidth(item_width);
		bool y = ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopStyleVar();

		ImGui::SameLine();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, GImGui->Style.ItemSpacing.y });
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);

		bool zb = ImGui::Button("Z", buttonSize);
		if (zb)
			values.z = resetvalue;

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();

		ImGui::SetNextItemWidth(item_width);
		bool z = ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopStyleVar();

		ImGui::Columns(1);
		ImGui::PopID();

		return x || xb || y || yb || z || zb;
	}

	static bool DrawVec2Control(const std::string& label, glm::vec2& values, const glm::vec2& min, const glm::vec2& max,
		const glm::vec2& resetvalue) {
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());
		float size = DrawLabel(label);
		
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
		float item_width = (size - buttonSize.x * 2 - GImGui->Style.ItemSpacing.x) / 2;
		
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, GImGui->Style.ItemSpacing.y });
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.2f, 0.1f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 1.0f, 0.3f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.2f, 0.1f, 1.0f });
		ImGui::PushFont(boldFont);

		bool xb = ImGui::Button("X", buttonSize);
		if (xb)
			values.x = resetvalue.x;
		
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		
		ImGui::SameLine();
		
		ImGui::SetNextItemWidth(item_width);
		bool x = ImGui::DragFloat("##X", &values.x, 0.1f, min.x, max.x, "%.2f");
		ImGui::PopStyleVar();
		
		ImGui::NextColumn();
		ImGui::SameLine();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, GImGui->Style.ItemSpacing.y });
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);

		bool yb = ImGui::Button("Y", buttonSize);
		if (yb)
			values.y = resetvalue.y;

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();

		ImGui::SetNextItemWidth(item_width);
		bool y = ImGui::DragFloat("##Y", &values.y, 0.1f, min.y, max.y, "%.2f");
		ImGui::PopStyleVar();

		ImGui::PopID();
		return x || xb || y || yb;
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

	static bool DrawBool(const std::string& label, bool& values) 
	{
		ImGui::PushID(label.c_str());
		ImGui::Text(label.c_str());
		ImGui::SameLine();

		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 25.0f);
		bool open = ImGui::Checkbox("##V", &values);
		ImGui::PopID();
		return open;
	}
	
	static bool DrawTextBox(const std::string& label, std::string& text)
	{
		ImGui::PushID(label.c_str());
		float size = DrawLabel(label);
		ImGui::SetNextItemWidth(size);

		char buffer[64];
		strncpy_s(buffer, sizeof(buffer), text.c_str(), sizeof(buffer));

		bool open = ImGui::InputText("##Class", buffer, sizeof(buffer));
		if (open)
			text = std::string(buffer);
		
		ImGui::PopID();
		return open;
	}

	static bool DrawCombo(const std::string& label, const char* strings[], uint32_t stringsSize, const char* currentString, int& index) {
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());
		float size = DrawLabel(label);
		ImGui::SetNextItemWidth(size);
		
		UI::ScopedStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f });

		bool open = false;
		if (ImGui::BeginCombo("##V", currentString)) 
		{
			open = true;
			for (uint32_t i = 0; i < stringsSize; i++) 
			{
				bool isSelected = currentString == strings[i];
				if (ImGui::Selectable(strings[i], isSelected)) 
				{
					currentString = strings[i];
					index = i;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}
		
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

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction function, bool deletable = false) {
		if (entity.HasComponent<T>()) {
			const ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed;
			auto& component = entity.GetComponent<T>();

			ImVec2 contentRegion = ImGui::GetContentRegionAvail();

			ImGui::PushID(name.c_str());

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 4 });
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 7.5f);
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeFlags, name.c_str());
			ImGui::PopStyleVar(2);
			
			bool removeComponent = false;
			if (deletable && ImGui::BeginPopupContextItem("ComponentSettings")) {
				if (ImGui::MenuItem("Remove Component"))
					removeComponent = true;
				
				ImGui::EndPopup();
			}

			if (open) {
				ImGuiStyle& style = ImGui::GetStyle();
				float indentation = ImGui::GetTreeNodeToLabelSpacing() - (style.FramePadding.x + style.ItemSpacing.x);
				ImGui::Unindent(indentation);
				
				function(component);
				ImGui::Separator();
				
				ImGui::Indent(indentation);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();

			ImGui::PopID();
		}
	}

	template<typename T>
	void SceneHierarchyPanel::DisplayAddComponentEntry(const std::string& entryName) {
		if (!m_SelectionContext.HasComponent<T>())
		{
			if (ImGui::MenuItem(entryName.c_str()))
			{
				m_SelectionContext.AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}

	static void UpdateChildProperties(Ref<Scene> scene, Entity entity, bool enabled)
	{
		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			if (b2Body* body = (b2Body*)entity.GetComponent<Rigidbody2DComponent>().RuntimeBody)
				body->SetEnabled(enabled);
		}
		
		auto& pcc = scene->GetEntityNode(entity.GetUUID());
		for (UUID id : pcc.Children)
		{
			Entity child = { id, entity };
			auto& prop = child.GetComponent<PropertiesComponent>();
			prop.Enabled = enabled;
			UpdateChildProperties(scene, child, enabled);
		}
	}
	
	static void UpdateChildProperties(Ref<Scene> scene, Entity entity, Ref<ProjectLayer> layer)
	{
		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			if (b2Fixture* fixture = (b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture)
			{
				b2Filter filter = fixture->GetFilterData();
				filter.categoryBits = layer->Identity;
				fixture->SetFilterData(filter);
			}
		}
		
		auto& pcc = scene->GetEntityNode(entity.GetUUID());
		for (UUID id : pcc.Children)
		{
			Entity child = { id, entity };
			auto& prop = child.GetComponent<PropertiesComponent>();
			prop.Layer = layer;
			UpdateChildProperties(scene, child, layer);
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity) {
		float buttonWidth = ImGui::CalcTextSize("Add Component").x + GImGui->Style.ItemSpacing.x * 2;

		if (entity.HasComponent<TagComponent>()) {
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			char buffer[256];
			strncpy_s(buffer, sizeof(buffer), tag.c_str(), sizeof(buffer));

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - buttonWidth - GImGui->Style.ItemSpacing.x);

			if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
				tag = std::string(buffer);
			}
		}

		ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - buttonWidth);
		
		if (ImGui::Button("Add Component", ImVec2{ buttonWidth, 0.0f }))
			ImGui::OpenPopup("Add Component");

		if (entity.HasComponent<PropertiesComponent>())
		{
			auto& prop = entity.GetComponent<PropertiesComponent>();

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 25.0f - GImGui->Style.ItemSpacing.x);
			if (ImGui::BeginCombo("##V", prop.Layer->Name.c_str()))
			{
				const ProjectConfig& pConfig = Project::GetActive()->GetConfig();

				for (auto [l, layer] : pConfig.Layers)
				{
					bool isSelected = prop.Layer == layer;
					
					if (ImGui::Selectable(layer->Name.c_str(), isSelected))
					{
						prop.Layer = pConfig.Layers.at(l);
						UpdateChildProperties(m_Context, entity, prop.Layer);
					}
					
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			ImGui::SameLine();

			if (ImGui::Checkbox("##Enabled", &prop.Enabled))
				UpdateChildProperties(m_Context, entity, prop.Enabled);
		}

		if (ImGui::BeginPopup("Add Component")) {
			DisplayAddComponentEntry<CameraComponent>("Camera");
			DisplayAddComponentEntry<ScriptComponent>("Script");

			DisplayAddComponentEntry<SpriteRendererComponent>("Sprite Renderer");
			DisplayAddComponentEntry<CircleRendererComponent>("Circle Renderer");
			DisplayAddComponentEntry<StringRendererComponent>("String Renderer");
			
			DisplayAddComponentEntry<Rigidbody2DComponent>("Rigidbody 2D");
			DisplayAddComponentEntry<BoxCollider2DComponent>("Box Collider 2D");
			DisplayAddComponentEntry<CircleColliderComponent>("Circle Collider 2D");
			
			ImGui::EndPopup();
		}

		DrawComponent<TransformComponent>("Transform", entity, [entity](auto& component) mutable {
			glm::vec3 rotation = degrees(component.Rotation);
			glm::vec3 translation = component.Translation;
			glm::vec3 scale = component.Scale;

			bool p = DrawVec3Control("Position", translation);
			bool r = DrawVec3Control("Rotation", rotation);
			bool s = DrawVec3Control("Scale", scale, 1.0f);
			
			ImGui::Spacing();

			component.Rotation = radians(rotation);
			component.Translation = translation;
			component.Scale = scale;

			if (p || r || s)
				entity.UpdateTransform();
		});

		DrawComponent<CameraComponent>("Camera", entity, [](auto& component) mutable {
			auto& camera = component.Camera;

			DrawBool("Primary", component.Primary);

			const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
			int index = (int)camera.GetProjectionType();
			if (DrawCombo("Projection", projectionTypeStrings, 2, projectionTypeStrings[index], index))
				camera.SetProjectionType((SceneCamera::ProjectionType)index);
			
			ImGui::Separator();
			ImGui::Spacing();

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic) {
				float Size = camera.GetOrthographicSize();
				if (DrawVec1Control("Size", Size, 0.0f, 0.0f, 0.0f, 0.1f)) {
					camera.SetOrthographicSize(Size);
				}

				float Near = camera.GetOrthoNearClip();
				if (DrawVec1Control("Near", Near)) {
					camera.SetOrthoNearClip(Near);
				}

				float Far = camera.GetOrthoFarClip();
				if (DrawVec1Control("Far", Far)) {
					camera.SetOrthoFarClip(Far);
				}

				DrawBool("Fixed Aspect Ratio", component.FixedAspectRatio);
			}

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective) {
				float fov = glm::degrees(camera.GetPerspectiveFOV());
				if (DrawVec1Control("FOV", fov)) {
					camera.SetPerspectiveFOV(glm::radians(fov));
				}

				float Near = camera.GetPerspectiveNearClip();
				if (DrawVec1Control("Near", Near)) {
					camera.SetPerspectiveNearClip(Near);
				}

				float Far = camera.GetPerspectiveFarClip();
				if (DrawVec1Control("Far", Far)) {
					camera.SetPerspectiveFarClip(Far);
				}
			}
		}, true);

		DrawComponent<ScriptComponent>("Script", entity, [entity, scene = m_Context](auto& component) mutable {
			bool classExists = ScriptEngine::EntityClassExists(component.ClassName);
			UI::ScopedStyleColor colour(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f), !classExists);
			
			if (DrawTextBox("Class", component.ClassName) && scene->IsRunning())
				ScriptEngine::CreateScriptInstance(entity);
			
			// FIELDS
			if (Ref<ScriptClass> scriptClass = ScriptEngine::GetEntityClass(component.ClassName))
			{
				ImGui::Separator();
				ImGui::Spacing();

				auto& fields = scriptClass->GetFields();
				ScriptFieldMap& map = ScriptEngine::GetScriptFieldMap(entity.GetUUID());

				for (auto& [name, class_field] : fields)
				{
					ScriptField& field = map.GetScriptField(name, entity);
					
					switch (field.Type)
					{
					case ScriptFieldType::Float:
					{
						auto data = field.GetValue<float>();
						if (DrawVec1Control(name.c_str(), data))
							field.SetValue(data);
						
						break;
					}
					case ScriptFieldType::Bool:
					{
						auto data = field.GetValue<bool>();
						if (DrawBool(name.c_str(), data))
							field.SetValue(data);
						
						break;
					}
					case ScriptFieldType::Int:
					{
						auto data = field.GetValue<int>();
						
						ImGui::PushID(name.c_str());
						float size = DrawLabel(name);
						ImGui::SetNextItemWidth(size);

						if (ImGui::DragInt("##V", &data))
							field.SetValue(data);
						
						ImGui::PopID();
						break;
					}
					case ScriptFieldType::Vector2:
					{
						auto data = field.GetValue<glm::vec2>();
						if (DrawVec2Control(name.c_str(), data))
							field.SetValue(data);
						
						break;
					}
					case ScriptFieldType::Vector3:
					{
						auto data = field.GetValue<glm::vec3>();
						if (DrawVec3Control(name.c_str(), data))
							field.SetValue(data);
						
						break;
					}
					case ScriptFieldType::Vector4:
					{
						auto data = field.GetValue<glm::vec4>();
						if (ImGui::DragFloat4(name.c_str(), value_ptr(data)))
							field.SetValue(data);
						
						break;
					}
					case ScriptFieldType::Entity:
					{
						UUID id = field.GetValueObject();
						std::string text = "None";
						
						if (id)
						{
							text = "Invalid Entity";
							if (Entity ent = { id, scene.get() })
								text = ent.GetName();
						}
						
						DrawLabel(name);
						if (ImGui::Button(text.c_str(), ImVec2{ ImGui::GetContentRegionAvail().x, 0 }))
							field.SetValue(nullptr);

						if (ImGui::BeginDragDropTarget())
						{
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
							{
								const UUID payloadID = *(const UUID*)payload->Data;
								field.SetValueEntity(payloadID);
							}
						}

						break;
					}
					case ScriptFieldType::Prefab:
					case ScriptFieldType::Font:
					case ScriptFieldType::Texture:
					case ScriptFieldType::Material:
					case ScriptFieldType::Asset:
					{
						AssetHandle handle = field.GetValueObject();
						const AssetMetadata& metadata = AssetManager::GetAssetMetadata(handle);
						
						std::string text = "None";
						if (metadata)
							text = metadata.RelativePath.string();
						
						DrawLabel(name);
						if (ImGui::Button(text.c_str(), ImVec2{ ImGui::GetContentRegionAvail().x, 0 }))
							field.SetValue(nullptr);

						if (ImGui::BeginDragDropTarget()) 
						{
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) 
							{
								const wchar_t* payloadPath = (const wchar_t*)payload->Data;
								std::filesystem::path path = payloadPath;

								AssetHandle handle = AssetManager::CreateAsset(path);
								field.SetValueAsset(handle);
							}
						}
						
						break;
					}
					}
				}
			}
		}, true);

		DrawComponent<MaterialComponent>("Material", entity, [](auto& component) {
			const AssetMetadata& metadata = AssetManager::GetAssetMetadata(component.Material);
			std::string text = metadata.RelativePath.empty() ? "Default" : metadata.RelativePath.string();

			DrawLabel("Material");
			if (ImGui::Button(text.c_str(), ImVec2{ ImGui::GetContentRegionAvail().x, 0 }))
				component.Material = NULL;

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* payloadPath = (const wchar_t*)payload->Data;
					std::filesystem::path path = payloadPath;

					if (AssetManager::GetTypeFromExtension(path.extension().string()) == AssetType::Material)
						component.Material = AssetManager::CreateAsset(path);
				}
			}

			Ref<Material> material = AssetManager::GetAsset<Material>(component.Material);
			bool isDefault = !material;

			if (isDefault)
				material = CreateRef<Material>();
			
			bool colour = DrawColourEdit("Colour", material->Colour);
			bool tiling = DrawVec1Control("Tiling", material->Tiling);
			
			DrawLabel("Texture");
			text = "None";
			if (material->Texture)
			{
				auto& metadata = AssetManager::GetAssetMetadata(material->Texture->Handle);
				if (metadata)
					text = metadata.RelativePath.string();
			}

			bool texture = false;
			if (ImGui::Button(text.c_str(), ImVec2{ ImGui::GetContentRegionAvail().x, 0 }))
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
				if (isDefault)
					component.Material = AssetManager::CreateMemoryAsset(material);
				
				const AssetMetadata& data = AssetManager::GetAssetMetadata(component.Material);

				MaterialSerializer serializer(material);
				serializer.Serialize(data.Path.string());
			}
		}, false);

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [entity](auto& component) mutable {
			glm::vec2 maxCellSize = glm::vec2(0.0f);

			if (AssetHandle mat = entity.GetComponent<MaterialComponent>().Material)
			{
				if (Ref<Material> material = AssetManager::GetAsset<Material>(mat))
				{
					if (Ref<Texture2D> texture = material->Texture)
					{
						maxCellSize = glm::vec2(texture->GetWidth(), texture->GetHeight());
						component.SubTextureCellSize = glm::min(component.SubTextureCellSize, maxCellSize);
					}
				}
			}
			
			DrawVec2Control("Offset", component.SubTextureOffset);
			DrawVec2Control("Cell Size", component.SubTextureCellSize, glm::vec2(0.1f), maxCellSize, maxCellSize);
			DrawVec2Control("Cell Number", component.SubTextureCellNum, glm::vec2(0.1f), glm::vec2(0.0f), glm::vec2(1.0f));
		}, true);

		DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](auto& component) {
			DrawVec1Control("Thickness", component.Thickness, 0.0f, 1.0f, 1.0f, 0.01f);
			DrawVec1Control("Fade", component.Fade, 0.005f, 1.0f, 0.01f, 0.001f);
		}, true);

		DrawComponent<StringRendererComponent>("String Renderer", entity, [](auto& component) {
			float size = DrawLabel("Text");
			ImGui::SetNextItemWidth(size);
			ImGui::InputTextMultiline("##Text", &component.Text);
			
			size = DrawLabel("Font");
			ImGui::SetNextItemWidth(size);

			const AssetMetadata& metadata = AssetManager::GetAssetMetadata(component.FontHandle);
			std::string currentName = "Default";
			bool isFamily = false;

			if (metadata)
			{
				isFamily = metadata.Type == AssetType::FontFamily;
				currentName = metadata.RelativePath.string();
			}
			
			if (ImGui::BeginCombo("##Font", currentName.c_str()))
			{
				if (ImGui::Selectable("Default", currentName == "Default"))
					component.FontHandle = NULL;
				
				Array<AssetHandle> handles = AssetManager::GetAllAssetsWithType(AssetType::FontFamily);
				AssetManager::GetAllAssetsWithType(handles, AssetType::Font, false);

				for (uint32_t i = 0; i < handles.size(); i++)
				{
					const AssetMetadata& asset = AssetManager::GetAssetMetadata(handles[i]);
					std::string name = asset.RelativePath.string();

					bool isSelected = component.FontHandle == asset.Handle;

					if (ImGui::Selectable(name.c_str(), isSelected))
						component.FontHandle = asset.Handle;
					
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
			
			if (isFamily)
			{
				DrawBool("Italic", component.Italic);
				DrawBool("Bold", component.Bold);
			}
			
			DrawVec1Control("Kerning", component.Kerning);
			DrawVec1Control("Line Spacing", component.LineSpacing);

			DrawColourEdit("Colour", component.Colour);
		}, true);

		DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [scene = m_Context](auto& component) {
			int componentType = (int)component.Type;
			const char* BodyTypeStrings[] = { "Static", "Dynamic", "Kinenmatic"};
			const char* CurrentBodyTypeString = BodyTypeStrings[componentType];
			
			if (DrawCombo("Body Type", BodyTypeStrings, 3, CurrentBodyTypeString, componentType))
				component.Type = (Rigidbody2DComponent::BodyType)componentType;

			DrawBool("Fixed Rotation", component.FixedRotation);
			DrawBool("Is Trigger", component.Trigger);
		}, true);

		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component) {
			DrawVec2Control("Offset", component.Offset);
			DrawVec2Control("Size",   component.Size);
			DrawVec1Control("Density", component.Density, 0.01f, 0.0f, 1.0f);
			DrawVec1Control("Friction", component.Friction, 0.01f, 0.0f, 1.0f);
			DrawVec1Control("Restitution", component.Restitution, 0.01f, 0.0f, 1.0f);
			DrawVec1Control("Restitution Threshold", component.RestitutionThreshold, 0.01f, 0.0f);
			
			ImGui::PushID("Mask");
			float size = DrawLabel("Mask");
			ImGui::SetNextItemWidth(size);

			if (ImGui::BeginCombo("##V", "Collide With"))
			{
				const ProjectConfig& pConfig = Project::GetActive()->GetConfig();

				for (auto& [l, info] : pConfig.Layers)
				{
					ImGui::PushID(info->Name.c_str());
					ImGui::Text(info->Name.c_str());
					ImGui::SameLine();

					ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - 25.0f);
					bool canInteract = component.Mask & l;

					if (ImGui::Checkbox("##V", &canInteract))
					{
						if (canInteract)
							component.Mask = component.Mask | l;
						else
							component.Mask = component.Mask & ~l;
					}

					ImGui::PopID();
				}

				ImGui::EndCombo();
			}
			ImGui::PopID();
		}, true);

		DrawComponent<CircleColliderComponent>("Circle Collider", entity, [](auto& component) {
			DrawVec2Control("Offset", component.Offset);
			DrawVec1Control("Radius", component.Radius, 0.01f);
			DrawVec1Control("Density", component.Density, 0.01f, 0.0f, 1.0f);
			DrawVec1Control("Friction", component.Friction, 0.01f, 0.0f, 1.0f);
			DrawVec1Control("Restitution", component.Restitution, 0.01f, 0.0f, 1.0f);
			DrawVec1Control("Restitution Threshold", component.RestitutionThreshold, 0.01f, 0.0f);
			
			ImGui::PushID("Mask");
			float size = DrawLabel("Mask");
			ImGui::SetNextItemWidth(size);

			if (ImGui::BeginCombo("##V", "Collide With"))
			{
				const ProjectConfig& pConfig = Project::GetActive()->GetConfig();

				for (auto& [l, info] : pConfig.Layers)
				{
					ImGui::PushID(info->Name.c_str());
					ImGui::Text(info->Name.c_str());
					ImGui::SameLine();

					ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - 25.0f);
					bool canInteract = component.Mask & l;

					if (ImGui::Checkbox("##V", &canInteract))
					{
						if (canInteract)
							component.Mask = component.Mask | l;
						else
							component.Mask = component.Mask & ~l;
					}

					ImGui::PopID();
				}

				ImGui::EndCombo();
			}
			ImGui::PopID();
		}, true);
	}
}