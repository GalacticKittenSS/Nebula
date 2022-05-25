#include "nbpch.h"
#include "Graphics_Context.h"

#include "Renderer.h"
#include "Platform/OpenGl/OpenGL_Context.h"

namespace Nebula {
	Scope<GraphicsContext> GraphicsContext::Create(void* window) {
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:	NB_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateScope<OpenGL_Context>(static_cast<GLFWwindow*>(window));
		}

		NB_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}
}
