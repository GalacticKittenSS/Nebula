#pragma once

#include "Render_Command.h"
#include "Orthographic_Camera.h"
#include "Shader.h"

namespace Nebula {
	class Renderer {
	public:
		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();

		static void Submit(
			const std::shared_ptr<Shader>& shader,
			const std::shared_ptr<VertexArray>& vertexArray,
			const glm::mat4& transform = glm::mat4(1.0f)
		);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:
		struct SceneData {
			glm::mat4 ViewProjectMatrix;
		};

		static SceneData* m_Data;
	};
}