#pragma once

#include "Camera.h"
#include "Texture.h"
#include "GameObjects.h"

#define NB_QUAD 0
#define NB_TRI  1

namespace Nebula {
	class VertexArray;

	struct Vertex;
	struct RenderData;

	class Renderer2D {
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const mat4& transform);
		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();

		static void Flush(Ref<VertexArray> vertexArray, uint32_t IndexCount);

		//Primitives
		static void Draw(const uint32_t type, const vec4* vertexPos, const uint32_t vertexCount, const mat4& transform, Ref<Texture2D> texture = nullptr);

		static void DrawQuad(Sprite& quad, float tiling = 1.0f);
		static void DrawQuad(const mat4& matrix, const vec4& colour, float tiling = 1.0f);

		static void DrawTriangle(Sprite& tri, float tiling = 1.0f);
	private:
		static void FlushAndReset();
		static Vertex* Draw(Vertex* vertexPtr, const uint32_t vertexCount, const vec4* vertexPos, const RenderData& data, float tiling);
	};
}