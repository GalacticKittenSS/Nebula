#pragma once

#include "Camera.h"
#include "Texture.h"
#include "Fonts.h"

#include "Nebula/Scene/Entity.h"

#define NB_STRING	6
#define NB_RECT		5
#define NB_QUAD		4
#define NB_TRI		3
#define NB_LINE		2
#define NB_CIRCLE	0

namespace Nebula {
	class VertexArray;
	class VertexBuffer;

	struct Vertex;
	struct CircleVertex;
	
	class Renderer2D 
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform = glm::mat4(1.0f));
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();

		static void Draw(const uint32_t type, Entity& quad);
		static void Draw(const uint32_t type, const glm::mat4& transform, const glm::vec4& colour, const Ref<Texture2D> texture = nullptr, float tiling = 1.0f);
		static void Draw(const uint32_t type, const glm::vec4* vertexPos, glm::vec2* texCoords,
			const glm::mat4& transform, const glm::vec4& colour, Ref<Texture2D> texture, float tiling);

		//Primitives

		struct TextParams
		{
			glm::vec4 Colour { 1.0f };
			float Kerning = 0.0f;
			float LineSpacing = 0.0f;
		};
		static void DrawString(const std::string& text, Ref<Font> font,
			const glm::mat4& transform, const TextParams& params, uint32_t entityID = -1);
		static void DrawTri(const uint32_t vertexCount, const glm::vec4* vertexPos, glm::vec2* texCoords,
			const glm::mat4& transform, const glm::vec4& colour, Ref<Texture2D> texture = nullptr, float tiling = 1.0f, uint32_t entityID = -1);
		static void DrawQuad(const uint32_t vertexCount, const glm::vec4* vertexPos, glm::vec2* texCoords,
			const glm::mat4& transform, const glm::vec4& colour, Ref<Texture2D> texture = nullptr, float tiling = 1.0f, uint32_t entityID = -1);
		static void DrawCircle(const glm::mat4& transform, const glm::vec4& colour, const float thickness = 1.0f, const float fade = 0.005f, uint32_t entityID = -1);
		static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& colour, int entityID = -1);
	private:
		static void FlushAndReset();
		static float GetTextureIndex(const Ref<Texture2D>& texture);
	};
}
