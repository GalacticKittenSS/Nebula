#include "nbpch.h"
#include "OpenGL_RendererAPI.h"

#include <glad/glad.h>

namespace Nebula {
	void OpenGLMessageCallback(
		unsigned source,
		unsigned type,
		unsigned id,
		unsigned severity,
		int length,
		const char* message,
		const void* userParam) {
		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:         NB_CRITICAL(message); return;
			case GL_DEBUG_SEVERITY_MEDIUM:       NB_ERROR(message); return;
			case GL_DEBUG_SEVERITY_LOW:          NB_WARN(message); return;
			case GL_DEBUG_SEVERITY_NOTIFICATION: NB_TRACE(message); return;
		}
	}

	void OpenGL_RendererAPI::Init() {
		NB_PROFILE_FUNCTION();

#ifdef NB_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glEnable(GL_CULL_FACE);
		glEnable(GL_LINE_SMOOTH);
	}

	void OpenGL_RendererAPI::SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
		glViewport(x, y, width, height);
	}

	void OpenGL_RendererAPI::Clear() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGL_RendererAPI::SetClearColour(float r, float g, float b, float a) {
		glClearColor(r, g, b, a);
	}

	void OpenGL_RendererAPI::SetClearColour(const vec4& colour) {
		glClearColor(colour.r, colour.g, colour.b, colour.a);
	}

	void OpenGL_RendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) {
		vertexArray->Bind();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGL_RendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) {
		vertexArray->Bind();
		glDrawArrays(GL_LINES, 0, vertexCount);
	}

	void OpenGL_RendererAPI::SetLineWidth(float width) {
		glLineWidth(width);
	}
}