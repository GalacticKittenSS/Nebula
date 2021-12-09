#include "nbpch.h"
#include "FrameBuffer.h"

#include "Renderer.h"
#include "Platform/OpenGl/OpenGL_FrameBuffer.h"

namespace Nebula {
	Ref<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecification& specifications) {
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:	NB_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateScope<OpenGL_FrameBuffer>(specifications);
		}

		NB_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}
}