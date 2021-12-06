#pragma once

#include "Camera.h"
#include "Texture.h"
#include "GameObjects.h"

namespace Nebula {

	class Renderer2D {
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();

		static void Flush();

		//Primitives
		static void DrawQuad(Sprite& quad, float tiling = 1.0f);
		static void DrawTriangle(Sprite& tri, float tiling = 1.0f);

		struct stats {
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;
			uint32_t TriCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 4 + TriCount * 3; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6 + TriCount * 3; }
		};

		static stats GetStats();
		static void ResetStats();
	private:
		static void FlushAndReset();
	};
}