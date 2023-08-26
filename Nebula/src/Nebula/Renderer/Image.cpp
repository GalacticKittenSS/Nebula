#include "nbpch.h"
#include "Image.h"

#include "Renderer_API.h"

#include "Platform/Vulkan/Vulkan_Image.h"
#include "Platform/OpenGL/OpenGL_Image.h"

namespace Nebula
{
	Ref<Image2D> Image2D::Create(const ImageSpecification& specifications)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:	NB_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::Vulkan: return CreateRef<Vulkan_Image>(specifications);
		case RendererAPI::API::OpenGL: return CreateRef<OpenGL_Image>(specifications);
		}

		NB_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}
}