#include "Scene_Hierarchy.h"

#include "../../Nebula/Modules/glm/glm/glm/glm.hpp"
#include "../../Nebula/Modules/glm/glm/glm/gtc/matrix_transform.hpp"

namespace Nebula {
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene) {
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context) {
		m_Context = context;
	}

	void SceneHierarchyPanel::OnImGuiRender() {
		glm::perspective(45.0f, 16.0f / 9.0f, 1000.0f, -10.0f);

		ImGui::Begin("Scene Hierarchy");

		m_Context->m_Registry.each([&](auto entityID) {
			Entity entity{ entityID, m_Context.get() };
			DrawEntityNode(entity);
		});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectionContext = {};

		ImGui::End();

		ImGui::Begin("Properties");

		if (m_SelectionContext)
			DrawComponents(m_SelectionContext);

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity) {
		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = (m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0 | ImGuiTreeNodeFlags_OpenOnArrow;
		bool opened = ImGui::TreeNodeEx((void*)(uint32_t)entity, flags, tag.c_str());

		if (ImGui::IsItemClicked()) {
			m_SelectionContext = entity;
		}

		if (opened) {
			//Draw Children
			ImGui::TreePop();
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity) {
		if (entity.HasComponent<TagComponent>()) {
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());

			if (ImGui::InputText("Tag", buffer, sizeof(buffer))) {
				tag = std::string(buffer);
			}
		}

		if (entity.HasComponent<TransformComponent>()) {
			if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform")) {
				auto& transformComp = entity.GetComponent<TransformComponent>();

				if (ImGui::DragFloat3("Position", value_ptr(transformComp.Translation), 0.1f)) {
					transformComp.CalculateMatrix();
				}

				if (ImGui::DragFloat3("Rotation", value_ptr(transformComp.Rotation), 0.1f)) {
					transformComp.CalculateMatrix();
				}

				if (ImGui::DragFloat3("Size", value_ptr(transformComp.Scale), 0.1f)) {
					transformComp.CalculateMatrix();
				}

				ImGui::TreePop();
			}
		}

		if (entity.HasComponent<CameraComponent>()) {
			if (ImGui::TreeNodeEx((void*)typeid(CameraComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Camera")) {
				auto& cameraComp = entity.GetComponent<CameraComponent>();
				auto& camera = cameraComp.Camera;

				ImGui::Checkbox("Primary", &cameraComp.Primary);

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

					ImGui::Checkbox("Fixed Aspect Ratio", &cameraComp.FixedAspectRatio);
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

				ImGui::TreePop();
			}
		}

		if (entity.HasComponent<SpriteRendererComponent>()) {
			if (ImGui::TreeNodeEx((void*)typeid(SpriteRendererComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Sprite Renderer")) {
				auto& src = entity.GetComponent<SpriteRendererComponent>();
				ImGui::ColorEdit4("Colour", value_ptr(src.Colour));

				ImGui::TreePop();
			}
		}
	}
}