#include "nbpch.h"
#include "Buffer.h"

#include "Renderer.h"
#include "Platform/OpenGl/OpenGL_Buffer.h"

namespace Nebula {
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size) {
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:		NB_ASSERT(false, "RendererAPI::None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:		return CreateRef<OpenGL_VertexBuffer>(size);
		}

		NB_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size) {
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:		NB_ASSERT(false, "RendererAPI::None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:		return CreateRef<OpenGL_VertexBuffer>(vertices, size);
		}

		NB_ASSERT(false, "Unknown Renderer API!"); 
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count) {
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:		NB_ASSERT(false, "RendererAPI::None is not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:		return CreateRef<OpenGL_IndexBuffer>(indices, count);
		}

		NB_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}
}
