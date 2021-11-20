#pragma once

#include "Camera.h"
#include "Texture.h"
#include "Sprites.h"

namespace Nebula {
	class Renderer2D {
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();

		//Primitives
		static void Draw(Quad& quad, float tiling = 1.0f);
		static void Draw(Triangle& tri, float tiling = 1.0f);
	};
}