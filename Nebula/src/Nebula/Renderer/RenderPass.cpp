#include "nbpch.h"
#include "RenderPass.h"

#include "Renderer_API.h"

#include "Platform/Vulkan/Vulkan_RenderPass.h"

namespace Nebula
{
	Ref<RenderPass> RenderPass::Create(const RenderPassSpecifications& specification)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:	NB_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::Vulkan:	return CreateRef<Vulkan_RenderPass>(specification);
		}

		NB_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}
}