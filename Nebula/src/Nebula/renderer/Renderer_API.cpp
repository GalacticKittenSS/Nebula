#include "nbpch.h"
#include "Renderer_API.h"

#include "Platform/OpenGl/OpenGL_RendererAPI.h"

namespace Nebula {
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

	Scope<RendererAPI> RendererAPI::Create() {
		switch (s_API) {
			case RendererAPI::API::None:	NB_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:	return CreateScope<OpenGL_RendererAPI>();
		}

		NB_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}
}