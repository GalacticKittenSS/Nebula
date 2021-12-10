#include "nbpch.h"
#include "Scene.h"

#include "Components.h"
#include "Entity.h"

#include "Nebula/Renderer/Renderer2D.h"

namespace Nebula {
	Scene::Scene() {
		
	}

	Scene::~Scene() {

	}

	Entity Scene::CreateEntity(const std::string& name) {
		Entity entity =  { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity": name;
		return entity;
	}

	void Scene::Render() {
		Camera* mainCam = nullptr;
		mat4* mainCamTransform = nullptr;

		auto view = m_Registry.view<TransformComponent, CameraComponent>();
		for (auto entity : view) {
			auto& [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

			if (camera.Primary) {
				mainCam = &camera.Camera;
				mainCamTransform = &transform.Transform;
				break;
			}
		}

		if (mainCam) {
			Renderer2D::BeginScene(*mainCam, *mainCamTransform);

			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group) {
				auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawQuad(transform, sprite.Colour, 1.0f);
			}

			Renderer2D::EndScene();
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height) {
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view) {
			auto& cameraComponent = view.get<CameraComponent>(entity);
			
			if (!cameraComponent.FixedAspectRatio) {
				cameraComponent.Camera.SetViewPortSize(width, height);
			}
		}
	}
}