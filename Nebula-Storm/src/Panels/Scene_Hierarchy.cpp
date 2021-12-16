#include "Scene_Hierarchy.h"
#include <cstring>

#include "../../Nebula/Modules/imgui/src/imgui.cpp"

#ifdef _MSVC_LANG
	#define _CRT_SECURE_NO_WARNINGS
#endif

namespace Nebula {
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene) {
		SetContext(scene);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context) {
		m_Context = context;
		m_SelectionContext = {};
	}

	void SceneHierarchyPanel::OnImGuiRender() {
		ImGui::Begin("Scene Hierarchy");

		m_Context->m_Registry.each([&](auto entityID) {
			Entity entity{ entityID, m_Context.get() };
			DrawEntityNode(entity);
		});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectionContext = {};

		if (ImGui::BeginPopupContextWindow(0, 1, false)) {
			if (ImGui::MenuItem("Create Empty"))
				m_Context->CreateEntity("Empty Entity");

			ImGui::EndPopup();
		}

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

		if (ImGui::IsItemClicked())
			m_SelectionContext = entity;
		
		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Delete Selected Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (opened) {
			//Draw Children
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
			if (ImGui::MenuItem("Camera")) {
				if (!m_SelectionContext.HasComponent<CameraComponent>())
					m_SelectionContext.AddComponent<CameraComponent>();
				else
					NB_WARN("This entity already has the Camera Component!");
				ImGui::CloseCurrentPopup();
			}
			
			

			if (ImGui::MenuItem("Sprite Renderer")) {
				if (!m_SelectionContext.HasComponent<CameraComponent>())
					m_SelectionContext.AddComponent<SpriteRendererComponent>();
				else
					NB_WARN("This entity already has the Camera Component!");
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		DrawComponent<TransformComponent>("Transform", entity, [](auto& component) {
			vec3 rotation = degrees(component.Rotation);

			DrawVec3Control("Position", component.Translation);
			DrawVec3Control("Rotation", rotation);
			DrawVec3Control("Scale", component.Scale, 1.0f);

			component.Rotation = radians(rotation);
		});

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
		}, true);
	}
}