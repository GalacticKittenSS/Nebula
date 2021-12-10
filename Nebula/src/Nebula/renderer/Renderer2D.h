#pragma once

#include "Camera.h"
#include "Texture.h"
#include "GameObjects.h"

namespace Nebula {
	class VertexArray;

	struct Vertex {
		vec3 Position;
		vec4 Colour;
		vec2 TexCoord;
		float TexIndex;
		float TilingFactor;
	};

	class Renderer2D {
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const mat4& transform);
		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();

		static void Flush(Ref<VertexArray> vertexArray, uint32_t IndexCount);

		//Primitives
		static void DrawQuad(Sprite& quad, float tiling = 1.0f);
		static void DrawQuad(const mat4& matrix, const vec4& colour, float tiling = 1.0f);

		static void DrawTriangle(Sprite& tri, float tiling = 1.0f);
	private:
		static void FlushAndReset();
		static Vertex* Draw(Vertex* vertexPtr, const size_t vertexCount, vec4* vertexPos, Sprite& sprite, float tiling = 1.0f);
	};
}