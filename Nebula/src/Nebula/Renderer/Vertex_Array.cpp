#include "nbpch.h"
#include "Vertex_Array.h"

#include "Platform/OpenGl/OpenGL_VertexArray.h"
#include "Renderer.h"

namespace Nebula {
	Ref<VertexArray> VertexArray::Create() {
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:		NB_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:		return CreateRef<OpenGL_VertexArray>();
		}

		NB_ASSERT(false, "Unknow Renderer API!");
		return nullptr;
	}
}
