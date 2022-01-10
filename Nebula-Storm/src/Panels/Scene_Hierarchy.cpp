#include "Scene_Hierarchy.h"
#include <cstring>

#include "../../Nebula/Modules/imgui/src/imgui.cpp"

#include <filesystem>

namespace Nebula {
	extern const std::filesystem::path s_AssetPath;

	static void AddParent(UUID childID, Entity parentEntity, Scene* scene) {
		bool isSafeToDrop = true;
		if (childID == parentEntity.GetUUID())
			isSafeToDrop = false;

		auto& parent = parentEntity.GetComponent<ParentChildComponent>();

		Entity dropEnt{ childID, scene };
		auto& child = dropEnt.GetComponent<ParentChildComponent>();


		//Check if Dropped Entity is being dropped to parent of itself
		if (child.PrimaryParent == parentEntity.GetUUID())
			isSafeToDrop = false;

		//Check if Dropped Entity is being dropped to child of itself
		for (uint32_t i = 0; i < child.ChildrenCount; i++) {
			if (child[i] == parentEntity.GetUUID())
				isSafeToDrop = false; break;
		}


		//Go Ahead if safe
		if (isSafeToDrop) {
			UUID parentID = child.PrimaryParent;
			if (parentID) {
				Entity parent{ parentID, scene };
				parent.GetComponent<ParentChildComponent>().RemoveChild(childID);
			}

			parent.AddChild(childID);

			child.PrimaryParent = parentEntity.GetUUID();
		}
	}

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene) {
		SetContext(scene);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context) {
		m_Context = context;
		m_SelectionContext = {};
	}

	void SceneHierarchyPanel::OnImGuiRender() {
		ImGui::Begin("Scene Hierarchy");

		if (m_Context) {
			for (uint32_t n = 0; n < m_Context->m_SceneOrder.size(); n++) {
				Entity entity{ m_Context->m_SceneOrder[n], m_Context.get()};
				
				if (!entity.GetComponent<ParentChildComponent>().PrimaryParent) {
					DrawEntityNode(entity);

					if (m_SelectionContext == entity && ImGui::IsWindowFocused()) {
						if (ImGui::IsMouseClicked(0))
							m_MovedEntityIndex = n;

						int32_t n_next = m_MovedEntityIndex + int32_t(ImGui::GetMouseDragDelta(0).y / 24);
						
						if (n_next >= 0 && n_next < m_Context->m_SceneOrder.size() && n != n_next && !ImGui::IsAnyItemHovered())
							m_Context->m_SceneOrder.move(n, n_next);
					}
					
					if (ImGui::IsMouseReleased(0)) {
						ImGui::ResetMouseDragDelta(0);
						m_MovedEntityIndex = -1;
					}
				}
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
					}

					if (ImGui::MenuItem("Camera")) {
						auto& cam = m_Context->CreateEntity("Camera");
						cam.AddComponent<CameraComponent>();
					}

					ImGui::EndMenu();
				}

				ImGui::EndPopup();
			}

			if (ImGui::IsMouseReleased(0) && ImGui::IsWindowHovered()) {
				const ImGuiPayload* payload = ImGui::GetDragDropPayload();
				if (payload != nullptr) {
					Entity Ent{ *(const UUID*)payload->Data, m_Context.get() };
					auto& ParentComp = Ent.GetParentChild();

					if (ParentComp.PrimaryParent) {
						Entity{ ParentComp.PrimaryParent, m_Context.get() }.GetComponent<ParentChildComponent>().RemoveChild(*(const UUID*)payload->Data);
						
						UUID parentsParent = Entity{ ParentComp.PrimaryParent, m_Context.get() }.GetParentChild().PrimaryParent;
						ParentComp.PrimaryParent = parentsParent;
						
						if (parentsParent)
							Entity{ parentsParent, m_Context.get() }.GetParentChild().AddChild(Ent.GetUUID());
					}
				}
			}
		}
		
		m_HierarchyFocused = ImGui::IsWindowFocused();
		m_HierarchyHovered = ImGui::IsWindowHovered();

		ImGui::End();


		ImGui::Begin("Properties");

		if (m_SelectionContext)
			DrawComponents(m_SelectionContext);

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity) {
		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		bool opened = ImGui::TreeNodeEx((void*)(uint32_t)entity, flags, tag.c_str());

		if (ImGui::BeginDragDropSource()) {
			UUID entityID = entity.GetUUID();
			ImGui::SetDragDropPayload("ENTITY", &entityID, sizeof(uint64_t), ImGuiCond_Once);
			ImGui::EndDragDropSource();
		}
		
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
				AddParent(*(const UUID*)payload->Data, entity, m_Context.get());
			
			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsItemClicked())
			m_SelectionContext = entity;
		
		bool entityDeleted = false;
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
				for (uint32_t i = 0; i < comp.ChildrenIDs.size(); i++) {
					Entity child{ comp[i], m_Context.get() };
					DrawEntityNode(child);

					if (m_SelectionContext == child && ImGui::IsWindowFocused()) {
						if (ImGui::IsMouseClicked(0))
							m_MovedEntityIndex = i;

						int32_t n_next = m_MovedEntityIndex + int32_t(ImGui::GetMouseDragDelta(0).y / 22);
						

						if (n_next >= 0 && n_next < entity.GetParentChild().ChildrenIDs.size() && i != n_next && !ImGui::IsAnyItemHovered())
							entity.GetParentChild().ChildrenIDs.move(i, n_next);
					}

					if (ImGui::IsMouseReleased(0)) {
						ImGui::ResetMouseDragDelta(0);
						m_MovedEntityIndex = -1;
					}
				}
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

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.2f, 0.1f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 1.0f, 0.3f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.2f, 0.1f, 1.0f });
		ImGui::PushFont(boldFont);

		if (ImGui::Button("X", buttonSize))
			values.x = resetvalue;

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
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
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	static void DrawVec2Control(const std::string& label, vec2& values, const vec2& min = vec2(0.0f), const vec2& max = vec2(0.0f), const vec2& resetvalue = vec2(0.0f), float columnWidth = 100.0f) {
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.2f, 0.1f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 1.0f, 0.3f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.2f, 0.1f, 1.0f });
		ImGui::PushFont(boldFont);

		if (ImGui::Button("X", buttonSize))
			values.x = resetvalue.x;

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
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
		ImGui::DragFloat("##Y", &values.y, 0.1f, min.y, max.y, "%.2f");
		ImGui::PopItemWidth();
		
		ImGui::PopStyleVar();
		
		ImGui::Columns(1);

		ImGui::PopID();
	}
	
	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction function, bool deletable = false) {
		if (entity.HasComponent<T>()) {
			const ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed;
			auto& component = entity.GetComponent<T>();

			ImVec2 contentRegion = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeFlags, name.c_str());
			ImGui::PopStyleVar();
			ImGui::SameLine(contentRegion.x - lineHeight * 0.5f);

			if (ImGui::Button("+", { lineHeight, lineHeight }))
				ImGui::OpenPopup("ComponentSettings");

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings")) {
				if (deletable) {
					if (ImGui::MenuItem("Remove Component"))
						removeComponent = true;
				}

				ImGui::EndPopup();
			}

			if (open) {
				function(component);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();
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
			if (!m_SelectionContext.HasComponent<CameraComponent>()) {
				if (ImGui::MenuItem("Camera")) {
					m_SelectionContext.AddComponent<CameraComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			
			if (!m_SelectionContext.HasComponent<SpriteRendererComponent>()) {
				if (ImGui::MenuItem("Sprite Renderer")) {
					m_SelectionContext.AddComponent<SpriteRendererComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!m_SelectionContext.HasComponent<CircleRendererComponent>()) {
				if (ImGui::MenuItem("Circle Renderer")) {
					m_SelectionContext.AddComponent<CircleRendererComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!m_SelectionContext.HasComponent<Rigidbody2DComponent>()) {
				if (ImGui::MenuItem("Rigidbody 2D")) {
					m_SelectionContext.AddComponent<Rigidbody2DComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!m_SelectionContext.HasComponent<Box2DComponent>()) {
				if (ImGui::MenuItem("Box Collider 2D")) {
					m_SelectionContext.AddComponent<Box2DComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!m_SelectionContext.HasComponent<CircleColliderComponent>()) {
				if (ImGui::MenuItem("Circle Collider")) {
					m_SelectionContext.AddComponent<CircleColliderComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();
		if (m_ShowGlobal) {
			DrawComponent<TransformComponent>("Transform", entity, [](auto& component) {
				vec3 rotation = degrees(component.GlobalRotation);
				vec3 translation = component.GlobalTranslation;
				vec3 scale = component.GlobalScale;

				DrawVec3Control("Position", translation);
				DrawVec3Control("Rotation", rotation);
				DrawVec3Control("Scale", scale, 1.0f);

				component.SetDeltaTransform(translation - component.GlobalTranslation, radians(rotation) - component.GlobalRotation, scale - component.GlobalScale);
			});
		}
		else {
			DrawComponent<TransformComponent>("Transform", entity, [](auto& component) {
				vec3 rotation = degrees(component.LocalRotation);
				vec3 translation = component.LocalTranslation;
				vec3 scale = component.LocalScale;

				DrawVec3Control("Position", translation);
				DrawVec3Control("Rotation", rotation);
				DrawVec3Control("Scale", scale, 1.0f);

				component.SetDeltaTransform(translation - component.LocalTranslation, radians(rotation) - component.LocalRotation, scale - component.LocalScale);
			});
		}

		DrawComponent<CameraComponent>("Camera", entity, [](auto& component) {
			auto& camera = component.Camera;

			ImGui::Checkbox("Primary", &component.Primary);

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
				if (ImGui::DragFloat("Size", &Size)) {
					camera.SetOrthographicSize(Size);
				}

				float Near = camera.GetOrthoNearClip();
				if (ImGui::DragFloat("Near", &Near)) {
					camera.SetOrthoNearClip(Near);
				}

				float Far = camera.GetOrthoFarClip();
				if (ImGui::DragFloat("Far", &Far)) {
					camera.SetOrthoFarClip(Far);
				}

				ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
			}

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective) {
				float fov = degrees(camera.GetPerspectiveFOV());
				if (ImGui::DragFloat("FOV", &fov)) {
					camera.SetPerspectiveFOV(radians(fov));
				}

				float Near = camera.GetPerspectiveNearClip();
				if (ImGui::DragFloat("Near", &Near)) {
					camera.SetPerspectiveNearClip(Near);
				}

				float Far = camera.GetPerspectiveFarClip();
				if (ImGui::DragFloat("Far", &Far)) {
					camera.SetPerspectiveFarClip(Far);
				}
			}
		}, true);

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component) {
			ImGui::ColorEdit4("Colour", value_ptr(component.Colour));

			ImGui::Button("Texture", ImVec2(100.0f, 0.0f));

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
				ImGui::DragFloat("Texture Tiling Factor", &component.Tiling, 0.1f, 0.0f, 100.0f);
				
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

		DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](auto& component) {
			ImGui::ColorEdit4("Colour", value_ptr(component.Colour));
			ImGui::DragFloat("Thickness", &component.Thickness, 0.01f, 0.01f, 1.0f);
			ImGui::DragFloat("Fade", &component.Fade, 0.0025f, 0.01f, 1.0f);
		}, true);

		DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto& component) {
			const char* BodyTypeStrings[] = { "Static", "Dynamic", "Kinenmatic"};
			const char* CurrentBodyTypeString = BodyTypeStrings[(int)component.Type];

			if (ImGui::BeginCombo("Body Type", CurrentBodyTypeString)) {
				for (int i = 0; i < 2; i++) {
					bool isSelected = CurrentBodyTypeString == BodyTypeStrings[i];
					if (ImGui::Selectable(BodyTypeStrings[i], isSelected)) {
						CurrentBodyTypeString = BodyTypeStrings[i];
						component.Type = (Rigidbody2DComponent::BodyType)i;
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
		}, true);

		DrawComponent<Box2DComponent>("Box Collider 2D", entity, [](auto& component) {
			ImGui::DragFloat2("Offset", value_ptr(component.Offset));
			ImGui::DragFloat2("Size", value_ptr(component.Size));
			
			ImGui::DragFloat("Density",		&component.Density,		 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Friction",	&component.Friction,	 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
		}, true);

		DrawComponent<CircleColliderComponent>("Circle Collider", entity, [](auto& component) {
			ImGui::DragFloat2("Offset", value_ptr(component.Offset));
			ImGui::DragFloat("Radius", &component.Radius, 0.01f);
			ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
		}, true);
	}
}