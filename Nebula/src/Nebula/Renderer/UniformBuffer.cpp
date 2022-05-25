#include "nbpch.h"
#include "UniformBuffer.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGL_UniformBuffer.h"

namespace Nebula {

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    NB_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGL_UniformBuffer>(size, binding);
		}

		NB_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}
