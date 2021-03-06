#include "Scene_Hierarchy.h"
#include "../../Nebula/Modules/imgui/src/imgui.cpp"

#include <cstring>
#include <filesystem>

namespace Nebula {
	static float s_MaxTextLength = 0.0f;
	static float s_MaxTransformTextLength = 0.0f;
	static float s_Max = 400.0f;

	extern const std::filesystem::path s_AssetPath;

	struct RectData {
		UUID Parent;
		ImRect Rect;
		uint32_t index;
	};

	static bool DroppedIntoChildren(Entity child, Entity parentEntity) {
		if (parentEntity.GetUUID() == child.GetUUID())
			return true;

		UUID parent = parentEntity.GetParentChild().Parent;
		if (!parent)
			return false;

		return DroppedIntoChildren(child, Entity(parent, parentEntity));
	}

	static void AddParent(UUID childID, Entity parentEntity, Scene* scene) {
		auto& parent = parentEntity.GetComponent<ParentChildComponent>();

		Entity dropEnt{ childID, scene };
		auto& child = dropEnt.GetComponent<ParentChildComponent>();

		//Check if Dropped Entity is being dropped to parent of itself
		if (child.Parent == parentEntity.GetUUID())
			return;

		if (DroppedIntoChildren(dropEnt, parentEntity))
			return;

		//Go Ahead if safe
		UUID parentID = child.Parent;
		if (parentID) {
			Entity parent{ parentID, scene };
			parent.GetParentChild().RemoveChild(childID);
		}

		parent.AddChild(childID);
		
		child.Parent = parentEntity.GetUUID();
		UpdateChildrenAndTransform(parentEntity);
	}
	
	static void EntityPayload(Scene* currentScene) {
		const ImGuiPayload* payload = ImGui::GetDragDropPayload();
		if (!payload)
			return;
	
		Entity Ent{ *(const UUID*)payload->Data, currentScene };
		auto& ParentComp = Ent.GetParentChild();

		if (!ParentComp.Parent)
			return;

		Entity{ ParentComp.Parent, currentScene }.GetComponent<ParentChildComponent>().RemoveChild(*(const UUID*)payload->Data);

		UUID parentsParent = Entity{ ParentComp.Parent, currentScene }.GetParentChild().Parent;
		ParentComp.Parent = parentsParent;

		if (parentsParent)
			Entity{ parentsParent, currentScene }.GetParentChild().AddChild(Ent.GetUUID());
	}

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene) {
		SetContext(scene);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context) {
		m_Context = context;
		m_SelectionContext = {};
	}

	void SceneHierarchyPanel::OnImGuiRender() {
		Rects.clear();

		ImGui::Begin("Scene Hierarchy");

		DrawSceneHierarchy();

		m_HierarchyFocused = ImGui::IsWindowFocused();
		m_HierarchyHovered = ImGui::IsWindowHovered();

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
			RectData& data = Rects[i];
			if (!ImGui::IsMouseHoveringRect(data.Rect.Min, data.Rect.Max)) continue;

			if (ImGui::BeginDragDropTargetCustom(data.Rect, 1)) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY")) {
					const UUID child = *(const UUID*)payload->Data;
						
					if (data.Parent) {
						Entity parent = { data.Parent, m_Context.get() };
						AddParent(child, parent, m_Context.get());
						if (data.index > parent.GetParentChild().ChildrenIDs.size() - 1)
							data.index = parent.GetParentChild().ChildrenIDs.size() - 1;

						uint32_t childIndex = parent.GetParentChild().ChildrenIDs.FindIndex(child);
						
						if (data.index > childIndex && data.index != parent.GetParentChild().ChildrenIDs.size() - 1)
							data.index -= 1;

						parent.GetParentChild().ChildrenIDs.move(childIndex, data.index);
					} else {
						Entity childE = { child, m_Context.get() };
						if (childE.GetParentChild().Parent)
							Entity{ childE.GetParentChild().Parent, m_Context.get() }.GetParentChild().RemoveChild(child);
						childE.GetParentChild().Parent = NULL;
						CalculateGlobalTransform(childE);

						uint32_t childIndex = m_Context->m_SceneOrder.FindIndex(child);

						if (data.index > childIndex && data.index != m_Context->m_SceneOrder.size() - 1)
							data.index -= 1;

						m_Context->m_SceneOrder.move(childIndex, data.index);
					}
				}
				ImGui::EndDragDropTarget();
			}

			break;
		}

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectionContext = {};

		if (ImGui::BeginPopupContextWindow(0, 1, false)) {
			if (ImGui::BeginMenu("Create Entity")) {
				if (ImGui::MenuItem("Empty"))
					auto& entity = m_Context->CreateEntity("Entity");

				if (ImGui::MenuItem("Sprite")) {
					auto& sprite = m_Context->CreateEntity("Sprite");
					sprite.AddComponent<SpriteRendererComponent>();
					sprite.AddComponent<Rigidbody2DComponent>();
					sprite.AddComponent<BoxCollider2DComponent>();
				}

				if (ImGui::MenuItem("Circle")) {
					auto& sprite = m_Context->CreateEntity("Circle");
					sprite.AddComponent<CircleRendererComponent>();
					sprite.AddComponent<Rigidbody2DComponent>();
					sprite.AddComponent<CircleColliderComponent>();
				}

				if (ImGui::MenuItem("Camera")) {
					auto& cam = m_Context->CreateEntity("Camera");
					cam.AddComponent<CameraComponent>();
				}

				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		if (ImGui::IsMouseReleased(0) && ImGui::IsWindowHovered() && !ImGui::IsDragDropPayloadBeingAccepted())
			EntityPayload(m_Context.get());
	}

	void SceneHierarchyPanel::DrawArray(Array<UUID>& entities, bool showIfParent) {
		Array<UUID> drawn;
		for (uint32_t n = 0; n < entities.size(); n++) {
			Entity entity{ entities[n], m_Context.get() };

			if (entity.GetComponent<ParentChildComponent>().Parent && !showIfParent)
				continue;
			
			bool deleted;
			DrawEntityNode(entity, drawn.size() - 1, deleted);
			
			if (deleted)
				continue;

			drawn.push_back(entities[n]);
			entities.move(n, drawn.size() - 1);
		}

		if (drawn.size()) {
			Entity entity{ drawn[drawn.size() - 1], m_Context.get() };
			ImVec2 cursorPos = ImGui::GetCursorPos();
			ImVec2 elementSize = ImGui::GetItemRectSize();
			elementSize.x -= ImGui::GetStyle().FramePadding.x;
			elementSize.y = ImGui::GetStyle().FramePadding.y;
			cursorPos.y -= ImGui::GetStyle().FramePadding.y;
			ImVec2 windowPos = ImGui::GetCurrentWindow()->Pos;
			RectData data;
			data.Parent = entity.GetParentChild().Parent;
			data.Rect = ImRect(windowPos.x + cursorPos.x, windowPos.y + cursorPos.y, windowPos.x + cursorPos.x + elementSize.x, windowPos.y + cursorPos.y + elementSize.y);
			data.index = m_Context->m_SceneOrder.size() - 1;
			Rects.push_back(data);
		}
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity, uint32_t index, bool& entityDeleted) {
		ImVec2 cursorPos = ImGui::GetCursorPos();
		ImVec2 elementSize = ImGui::GetItemRectSize();
		elementSize.x -= ImGui::GetStyle().FramePadding.x;
		elementSize.y = ImGui::GetStyle().FramePadding.y;
		cursorPos.y -= ImGui::GetStyle().FramePadding.y;
		ImVec2 windowPos = ImGui::GetCurrentWindow()->Pos;
		RectData data;
		data.Parent = entity.GetParentChild().Parent;

		if (index == 0)
			elementSize.x = ImGui::GetContentRegionAvailWidth();

		data.Rect = ImRect(windowPos.x + cursorPos.x, windowPos.y + cursorPos.y, windowPos.x + cursorPos.x + elementSize.x, windowPos.y + cursorPos.y + elementSize.y);
		data.index = index;
		Rects.push_back(data);

		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = m_SelectionContext == entity ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding;

		if (entity.GetParentChild().ChildrenIDs.size() == 0)
			flags |= ImGuiTreeNodeFlags_Leaf;
		
		bool opened = ImGui::TreeNodeEx((void*)(uint32_t)entity, flags, tag.c_str());

		if (ImGui::IsItemClicked())
			m_SelectionContext = entity;

		if (ImGui::BeginDragDropSource()) {
			UUID entityID = entity.GetUUID();
			ImGui::SetDragDropPayload("ENTITY", &entityID, sizeof(uint64_t), ImGuiCond_Once);
			ImGui::Text(tag.c_str());
			ImGui::EndDragDropSource();
		}
		
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
				AddParent(*(const UUID*)payload->Data, entity, m_Context.get());
			
			ImGui::EndDragDropTarget();
		}

		entityDeleted = false;
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Delete Selected Entity"))
				entityDeleted = true;

			if (ImGui::BeginMenu("Create Entity")) {
				if (ImGui::MenuItem("Empty")) {
					auto& newEnt = m_Context->CreateEntity("Entity");
					AddParent(newEnt.GetUUID(), entity, m_Context.get());
				}

				if (ImGui::MenuItem("Sprite")) {
					auto& sprite = m_Context->CreateEntity("Sprite");
					sprite.AddComponent<SpriteRendererComponent>();
					AddParent(sprite.GetUUID(), entity, m_Context.get());
				}

				if (ImGui::MenuItem("Camera")) {
					auto& cam = m_Context->CreateEntity("Camera");
					cam.AddComponent<CameraComponent>();
					AddParent(cam.GetUUID(), entity, m_Context.get());
				}

				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		if (opened) {
			if (entity.HasComponent<ParentChildComponent>()) {
				auto& comp = entity.GetParentChild();
				DrawArray(comp.ChildrenIDs, true);
			}

			ImGui::TreePop();
		}
		
		if (entityDeleted) {
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
		}
	}

	static void DrawVec3Control(const std::string& label, vec3& values, float resetvalue = 0.0f, float columnWidth = 100.0f) {
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		
		ImGui::PushID(label.c_str());
		ImGui::Text(label.c_str());
		ImGui::SameLine();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		float tl = ImGui::GetCursorPosX();
		if (tl > s_MaxTextLength)
			s_MaxTextLength = tl;

		float size = ImGui::GetWindowContentRegionMax().x - s_MaxTextLength - 20.0f;
		if (size > s_Max)
			size = s_Max;

		float width = (size - (buttonSize.x * 3 + ImGui::GetStyle().ItemSpacing.x * 5)) / 3;
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - size);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.2f, 0.1f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 1.0f, 0.3f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.2f, 0.1f, 1.0f });
		ImGui::PushFont(boldFont);
		
		if (ImGui::Button("X", buttonSize))
			values.x = resetvalue;
		
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(width);
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();


		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		
		if (ImGui::Button("Y", buttonSize))
			values.y = resetvalue;
		
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		
		ImGui::SameLine();
		ImGui::SetNextItemWidth(width);
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();
		
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		
		if (ImGui::Button("Z", buttonSize))
			values.z = resetvalue;
		
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		
		ImGui::SameLine();
		ImGui::SetNextItemWidth(width);
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopStyleVar();

		ImGui::PopItemWidth();
		ImGui::Columns(1);
		ImGui::PopID();
	}

	static bool DrawVec3Transform(const std::string& label, vec3& values, float resetvalue = 0.0f, float columnWidth = 100.0f) {
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());
		ImGui::Text(label.c_str());
		ImGui::SameLine();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		float tl = ImGui::GetCursorPosX();
		if (tl > s_MaxTransformTextLength)
			s_MaxTransformTextLength = tl;

		float size = ImGui::GetWindowContentRegionMax().x - s_MaxTransformTextLength - 20.0f;
		if (size > s_Max)
			size = s_Max;

		float width = (size - buttonSize.x * 3) / 3;
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - size);

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
		ImGui::SetNextItemWidth(width);
		bool x = ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

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
		ImGui::SetNextItemWidth(width);
		bool y = ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

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
		ImGui::SetNextItemWidth(width);
		bool z = ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
		return x || xb || y || yb || z || zb;
	}

	static void DrawVec2Control(const std::string& label, vec2& values, const vec2& min = vec2(0.0f), const vec2& max = vec2(0.0f), const vec2& resetvalue = vec2(0.0f), float columnWidth = 100.0f) {
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());
		ImGui::Text(label.c_str());
		ImGui::SameLine();
		
		ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
		
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		float tl = ImGui::GetCursorPosX();
		if (tl > s_MaxTextLength)
			s_MaxTextLength = tl;

		float size = ImGui::GetWindowContentRegionMax().x - s_MaxTextLength - 20.0f;
		if (size > s_Max)
			size = s_Max;

		float width = (size - (buttonSize.x * 2 + ImGui::GetStyle().ItemSpacing.x * 3)) / 2;
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - size);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.2f, 0.1f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 1.0f, 0.3f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.2f, 0.1f, 1.0f });
		ImGui::PushFont(boldFont);
		
		if (ImGui::Button("X", buttonSize))
			values.x = resetvalue.x;
		
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		
		ImGui::SameLine();
		ImGui::SetNextItemWidth(width);
		ImGui::DragFloat("##X", &values.x, 0.1f, min.x, max.x, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);

		if (ImGui::Button("Y", buttonSize))
			values.y = resetvalue.y;

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(width);
		ImGui::DragFloat("##Y", &values.y, 0.1f, min.y, max.y, "%.2f");
		ImGui::PopItemWidth();
		
		ImGui::PopID();
	}

	static bool DrawVec1Control(const std::string& label, float& values, const float& min = 0.0f, const float& max = 0.0f, const float& resetvalue = 0.0f, float step = 0.1f, float columnWidth = 100.0f) {
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());
		ImGui::Text(label.c_str());
		ImGui::SameLine();
		
		float tl = ImGui::GetCursorPosX();
		if (tl > s_MaxTextLength)
			s_MaxTextLength = tl;

		float size = ImGui::GetWindowContentRegionMax().x - s_MaxTextLength - 20.0f;
		if (size > s_Max)
			size = s_Max;
		
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - size);
		ImGui::SetNextItemWidth(size);
		bool open = ImGui::DragFloat("##V", &values, step, min, max, "%.2f");
		ImGui::PopID();
		return open;
	}

	static bool DrawBool(const std::string& label, bool& values) {
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());
		ImGui::Text(label.c_str());
		ImGui::SameLine();

		float tl = ImGui::GetCursorPosX();
		if (tl > s_MaxTextLength)
			s_MaxTextLength = tl;

		float size = ImGui::GetWindowContentRegionMax().x - s_MaxTextLength - 20.0f;
		if (size > s_Max)
			size = s_Max;

		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - size);
		bool open = ImGui::Checkbox("##V", &values);
		ImGui::PopID();
		return open;
	}

	static bool DrawCombo(const std::string& label, const char* strings[], uint32_t stringsSize, const char* currentString, int& index) {
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());
		ImGui::Text(label.c_str());
		ImGui::SameLine();

		float tl = ImGui::GetCursorPosX();
		if (tl > s_MaxTextLength)
			s_MaxTextLength = tl;

		float size = ImGui::GetWindowContentRegionMax().x - s_MaxTextLength - 20.0f;
		if (size > s_Max)
			size = s_Max;

		bool open = false;
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - size);
		ImGui::SetNextItemWidth(size);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f });
		if (ImGui::BeginCombo("##V", currentString)) {
			open = true;
			for (int i = 0; i < stringsSize; i++) {
				bool isSelected = currentString == strings[i];
				if (ImGui::Selectable(strings[i], isSelected)) {
					currentString = strings[i];
					index = i;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}
		ImGui::PopStyleColor();

		ImGui::PopID();
		return open;
	}
	
	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction function, bool deletable = false) {
		if (entity.HasComponent<T>()) {
			const ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed;
			auto& component = entity.GetComponent<T>();

			ImVec2 contentRegion = ImGui::GetContentRegionAvail();

			ImGui::PushID(name.c_str());

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeFlags, name.c_str());
			ImGui::PopStyleVar();
			ImGui::SameLine(contentRegion.x - lineHeight * 0.5f);

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f });
			if (ImGui::Button("+", { lineHeight, lineHeight }))
				ImGui::OpenPopup("ComponentSettings");
			ImGui::PopStyleColor();
			
			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings")) {
				if (deletable) {
					if (ImGui::MenuItem("Remove Component"))
						removeComponent = true;
				}

				ImGui::EndPopup();
			}

			if (open) {
				function(component, entity);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();

			ImGui::PopID();
		}
	}

	static int RigidbodyFilterToIndex(int16_t filter) {
		switch (filter)
		{
			case 1: return 0;     case 2: return 1;     case 4: return 2;      case 8: return 3;
			case 16: return 4;    case 32: return 5;    case 64: return 6;     case 128: return 7;
			case 256: return 8;   case 512: return 9;   case 1024: return 10;  case 2048: return 11;
			case 4096: return 12; case 8192: return 13; case 16384: return 14; case 32768: return 15;
		}

		return 0;
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

	void SceneHierarchyPanel::DrawComponents(Entity entity) {
		if (entity.HasComponent<TagComponent>()) {
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strncpy(buffer, tag.c_str(), sizeof(buffer));

			if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
				tag = std::string(buffer);
			}
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("Add Component");

		if (ImGui::BeginPopup("Add Component")) {
			DisplayAddComponentEntry<CameraComponent>("Camera");
			DisplayAddComponentEntry<SpriteRendererComponent>("Sprite Renderer");
			DisplayAddComponentEntry<CircleRendererComponent>("Circle Renderer");
			DisplayAddComponentEntry<Rigidbody2DComponent>("Rigidbody 2D");
			DisplayAddComponentEntry<BoxCollider2DComponent>("Box Collider 2D");
			DisplayAddComponentEntry<CircleColliderComponent>("Circle Collider 2D");

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();
		
		DrawComponent<TransformComponent>("Transform", entity, [](auto& component, Entity entity) {
			vec3 rotation = degrees(component.Rotation);
			vec3 translation = component.Translation;
			vec3 scale = component.Scale;
			bool p = DrawVec3Transform("Position", translation);
			bool r = DrawVec3Transform("Rotation", rotation);
			bool s = DrawVec3Transform("Scale", scale, 1.0f);
			ImGui::Spacing();

			component.Rotation = radians(rotation);
			component.Translation = translation;
			component.Scale = scale;

			if (p || r || s)
				UpdateChildrenAndTransform(entity);
		});
	
		DrawComponent<CameraComponent>("Camera", entity, [](auto& component, Entity entity) {
			auto& camera = component.Camera;

			DrawBool("Primary", component.Primary);

			const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
			const char* currentProjString = projectionTypeStrings[(int)camera.GetProjectionType()];

			if (ImGui::BeginCombo("Projection", currentProjString)) {
				for (int i = 0; i < 2; i++) {
					bool isSelected = currentProjString == projectionTypeStrings[i];
					if (ImGui::Selectable(projectionTypeStrings[i], isSelected)) {
						currentProjString = projectionTypeStrings[i];
						camera.SetProjectionType((SceneCamera::ProjectionType)i);
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic) {
				float Size = camera.GetOrthographicSize();
				if (DrawVec1Control("Size", Size)) {
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
				float fov = degrees(camera.GetPerspectiveFOV());
				if (DrawVec1Control("FOV", fov)) {
					camera.SetPerspectiveFOV(radians(fov));
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

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component, Entity entity) {
			ImGui::ColorEdit4("Colour", value_ptr(component.Colour));

			std::string text;
			if (component.Texture == nullptr)
				text = "Texture";
			else
				text = component.Texture->GetPath();
			
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, ImGui::GetStyle().ItemSpacing.y));
			ImGui::BeginGroup();
			
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f });
			ImGui::Button(text.c_str(), ImVec2(ImGui::GetContentRegionAvailWidth() - 40.0f, 0.0f));
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f });
			if (ImGui::Button("X", ImVec2(40.0f, 0.0f)))
				component.Texture = nullptr;
			ImGui::PopStyleColor();
			
			ImGui::EndGroup();
			ImGui::PopStyleVar();

			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path texturePath = std::filesystem::path(s_AssetPath) / path;
					Ref<Texture2D> texture = Texture2D::Create(texturePath.string());
					if (texture->IsLoaded()) {
						component.Texture = texture;
						component.SubTextureCellSize = { (float)texture->GetWidth(), (float)texture->GetHeight() };
					}
					else
						NB_WARN("Could not load texture {0}", texturePath.filename().string());
				}
				ImGui::EndDragDropTarget();
			}

			if (component.Texture != nullptr) {
				DrawVec1Control("Texture Tiling Factor", component.Tiling, 0.1f, 0.0f, 100.0f);
				
				if (ImGui::TreeNodeEx("Sub Texture")) {
					vec2 textureSize = { (float)component.Texture->GetWidth(), (float)component.Texture->GetHeight() };
					vec2 maxOffset = textureSize - component.SubTextureCellSize * component.SubTextureCellNum;

					if (component.SubTextureOffset > maxOffset)
						component.SubTextureOffset = maxOffset;

					if (component.SubTextureOffset < vec2(0.0f))
						component.SubTextureOffset = vec2(0.0f);

					vec2 maxCellNum = textureSize / component.SubTextureCellSize;

					if (component.SubTextureCellNum > maxCellNum)
						component.SubTextureCellNum = maxCellNum;

					DrawVec2Control("Offset", component.SubTextureOffset, vec2(0.0f), maxOffset != vec2(0.0f) ? maxOffset : vec2(0.001f));
					DrawVec2Control("Cell Size", component.SubTextureCellSize, vec2(0.1f), textureSize, textureSize);
					DrawVec2Control("Cell Number", component.SubTextureCellNum, vec2(0.01f), maxCellNum);
					ImGui::TreePop();
				}
			}
		}, true);

		DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](auto& component, Entity entity) {
			ImGui::ColorEdit4("Colour", value_ptr(component.Colour));
			DrawVec1Control("Thickness", component.Thickness, 0.01f, 0.01f, 1.0f);
			DrawVec1Control("Fade", component.Fade, 0.0025f, 0.01f, 1.0f);
		}, true);

		DrawComponent<StringRendererComponent>("String Renderer", entity, [](auto& component, Entity entity) {
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strncpy(buffer, component.Text.c_str(), sizeof(buffer));

			ImGui::Text("Text");
			ImGui::SameLine();

			float tl = ImGui::GetCursorPosX();
			if (tl > s_MaxTextLength)
				s_MaxTextLength = tl;

			float size = ImGui::GetWindowContentRegionMax().x - s_MaxTextLength - 20.0f;
			if (size > s_Max)
				size = s_Max;

			bool open = false;
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - size);
			ImGui::SetNextItemWidth(size);

			if (ImGui::InputText("##Text", buffer, sizeof(buffer)))
				component.Text = std::string(buffer);
			
			 false;

			const char* fontStrings[] = StringRenderFontTypeStrings;
			bool font = DrawCombo("Font", fontStrings, 3, fontStrings[component.FontTypeIndex], component.FontTypeIndex);
			bool italic = DrawBool("Italic", component.Italic);
			bool bold = DrawBool("Bold", component.Bold);
			bool resolution = DrawVec1Control("Resolution", component.Resolution, 8.0f, 512.0f, 96.0f, 8.0f);
			
			ImGui::ColorEdit4("Colour", value_ptr(component.Colour));

			if (font || italic || bold || resolution) {
				delete component.Ft;
				component.InitiateFont();
			}
		}, true);

		DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto& component, Entity entity) {
			int componentType = (int)component.Type;
			const char* BodyTypeStrings[] = { "Static", "Dynamic", "Kinenmatic"};
			const char* CurrentBodyTypeString = BodyTypeStrings[componentType];
			
			if (DrawCombo("Body Type", BodyTypeStrings, 3, CurrentBodyTypeString, componentType))
				component.Type = (Rigidbody2DComponent::BodyType)componentType;

			DrawBool("Fixed Rotation", component.FixedRotation);

			DrawVec1Control("Density", component.Density, 0.01f, 0.0f, 1.0f);
			DrawVec1Control("Friction", component.Friction, 0.01f, 0.0f, 1.0f);
			DrawVec1Control("Restitution", component.Restitution, 0.01f, 0.0f, 1.0f);
			DrawVec1Control("Restitution Threshold", component.RestitutionThreshold, 0.01f, 0.0f);
		}, true);

		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component, Entity entity) {
			DrawVec2Control("Offset", component.Offset);
			DrawVec2Control("Size",   component.Size);

			static const char* filterTypeStrings[] = {
				"A", "B", "C", "D",
				"E", "F", "G", "H",
				"I", "J", "K", "L",
				"M", "N", "O", "P"
			};
			
			int categoryIndex = RigidbodyFilterToIndex((int)component.Category);
			const char* CurrentCategoryString = filterTypeStrings[categoryIndex];

			if (DrawCombo("Category", filterTypeStrings, 16, CurrentCategoryString, categoryIndex)) {
				int newCategory = pow(2, categoryIndex);
				component.UpdateFilters(newCategory, newCategory);
			}
		}, true);

		DrawComponent<CircleColliderComponent>("Circle Collider", entity, [](auto& component, Entity entity) {
			DrawVec2Control("Offset", component.Offset);
			DrawVec1Control("Radius", component.Radius, 0.01f);

			static const char* filterTypeStrings[] = {
				"A", "B", "C", "D",
				"E", "F", "G", "H",
				"I", "J", "K", "L",
				"M", "N", "O", "P"
			};

			int categoryIndex = RigidbodyFilterToIndex((int)component.Category);
			const char* CurrentCategoryString = filterTypeStrings[categoryIndex];

			if (DrawCombo("Layer", filterTypeStrings, 16, CurrentCategoryString, categoryIndex)) {
				int newCategory = pow(2, categoryIndex);
				component.UpdateFilters(newCategory, newCategory);
			}
		}, true);
	}
}