#pragma once

#include "Components.h"

#include "Nebula/Renderer/Camera.h"
#include "Nebula/Renderer/Framebuffer.h"
#include "Nebula/Renderer/Pipeline.h"
#include "Nebula/Renderer/UniformBuffer.h"
#include "Nebula/Renderer/Texture.h"
#include "Nebula/Renderer/Material.h"

namespace Nebula
{
	class Scene;

	class SceneRenderer
	{
	public:
		struct Settings
		{
			bool ShowColliders = false;
			bool ShowSky = true;

			static const uint32_t MaxSprites = 10000;
			static const uint32_t MaxTextureSlots = 32;
			static const uint32_t MaxVertices = MaxSprites * 4;
			static const uint32_t MaxIndices = MaxSprites * 6;
		};
		Settings m_Settings;
	public:
		static void Setup();
		static void Shutdown();

		SceneRenderer();
		
		void SetContext(Ref<Scene> scene) { m_Context = scene; }

		void Render(const EditorCamera& camera);
		void Render(const Camera& camera, const glm::mat4& transform);
		void Resize(uint32_t width, uint32_t height);

		glm::vec2 GetFramebufferSize();

		Ref<Image2D> GetFinalImage();
		int ReadImage(uint32_t x, uint32_t y);
	private:
		void RenderSprite(const glm::mat4& transform, Ref<Material> mat, const SpriteRendererComponent& sprite, int entityID);
		void RenderCircle(const glm::mat4& transform, Ref<Material> mat, const CircleRendererComponent& circle, int entityID);
		void RenderString(const glm::mat4& transform, Ref<Font> font, const StringRendererComponent& string, int entityID);

		void RenderCircleCollider(glm::mat4& transform, const CircleColliderComponent& circleCollider, const glm::vec3& projectionCollider, int entityID);
		void RenderBoxCollider(glm::mat4& transform, const BoxCollider2DComponent& boxCollider, float zIndex, int entityID);

		void SkyPrePass(glm::vec3 position);
		void GeometryPrePass();
		void ColliderPrePass(glm::vec3 forward);
		
		void SkyPass();
		void GeometryPass();
		void ColliderPass();

		void FlushAndReset();
		float GetTextureIndex(const Ref<Texture2D>& texture);
	private:
		struct RenderData
		{
			Ref<FrameBuffer> Frambuffer;
			Ref<RenderPass> SkyPass;
			Ref<RenderPass> GeometryPass;
			Ref<RenderPass> ColliderPass;
			
			Ref<Shader>		TextureShader;
			Ref<Shader>		 CircleShader;
			Ref<Shader>		   LineShader;
			Ref<Shader>		   TextShader;

			Ref<Pipeline>	TexturePipeline;
			Ref<Pipeline>	 CirclePipeline;
			Ref<Pipeline>	   LinePipeline;
			Ref<Pipeline>	   TextPipeline;

			struct CameraData
			{
				glm::mat4 ViewProjection;
			};
			CameraData CameraBuffer;
			Ref<UniformBuffer> CameraUniformBuffer;
		};
		RenderData m_Data;

		Ref<Scene> m_Context;
	};
}