#include "nbpch.h"
#include "Shader.h"

#include "Renderer.h"
#include "Platform/OpenGl/OpenGL_Shader.h"

namespace Nebula {
	Shader* Shader::Create(const std::string& vertSrc, const std::string& fragSrc) {
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:	NB_ASSERT(false, "RendererAPI::None is currently not supported!");
			case RendererAPI::API::OpenGL:	return new OpenGL_Shader(vertSrc, fragSrc);
		}

		NB_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}