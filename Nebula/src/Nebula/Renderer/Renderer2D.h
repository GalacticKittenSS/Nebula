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
	struct LineVertex;
	
	class Renderer2D {
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const mat4& transform = mat4(1.0f));
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();

		static void SetBackCulling(bool);

		static void Draw(const uint32_t type, Entity& quad);
		static void Draw(const uint32_t type, const mat4& transform, const vec4& colour, const Ref<Texture2D> texture = nullptr, float tiling = 1.0f);
		static void Draw(const uint32_t type, const vec4* vertexPos, vec2* texCoords,
			const mat4& transform, const vec4& colour, Ref<Texture2D> texture, float tiling);

		//Primitives
		static void DrawString(const std::string& text, Font* font,
			const mat4& transform, const vec4& colour, uint32_t entityID = -1);
		static void DrawTri(const uint32_t vertexCount, const vec4* vertexPos, vec2* texCoords,
			const mat4& transform, const vec4& colour, Ref<Texture2D> texture = nullptr, float tiling = 1.0f, uint32_t entityID = -1);
		static void DrawQuad(const uint32_t vertexCount, const vec4* vertexPos, vec2* texCoords,
			const mat4& transform, const vec4& colour, Ref<Texture2D> texture = nullptr, float tiling = 1.0f, uint32_t entityID = -1);
		static void DrawCircle(const mat4& transform, const vec4& colour, const float thickness = 1.0f, const float fade = 0.005f, uint32_t entityID = -1);
		static void DrawLine(const vec3& p0, const vec3& p1, const vec4& colour, int entityID = -1);
	private:
		static void FlushAndReset();
		static float GetTextureIndex(const Ref<Texture2D>& texture);
		static Vertex* CalculateVertexData(Vertex* vertexPtr, const uint32_t vertexCount, const vec4* vertexPos,
			const mat4& transform, const vec4& colour, Ref<Texture2D> texture, vec2* texCoord, float tiling, uint32_t entityID);
		static CircleVertex* CalculateVertexData(CircleVertex* vertexPtr, const uint32_t vertexCount, const vec4* vertexPos,
			const mat4& transform, const vec4& colour, float thickness, float fade, uint32_t entityID);
	};
}
