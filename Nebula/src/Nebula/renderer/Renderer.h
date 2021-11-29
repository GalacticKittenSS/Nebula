#pragma once

#include "Render_Command.h"
#include "Camera.h"
#include "Shader.h"

#include "Nebula/Maths/Maths.h"

namespace Nebula {
	class Renderer {
	public:
		static void Init();
		static void Shutdown();

		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();

		static void Submit(
			const Ref<Shader>& shader,
			const Ref<VertexArray>& vertexArray,
			const mat4& transform = mat4(1.0f)
		);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:
		struct SceneData {
			mat4 ViewProjectMatrix;
		};

		static Scope<SceneData> s_Data;
	};
}