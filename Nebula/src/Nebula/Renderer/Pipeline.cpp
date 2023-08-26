#include "nbpch.h"
#include "Pipeline.h"

#include "Renderer_API.h"

#include "Platform/Vulkan/Vulkan_Pipeline.h"

namespace Nebula
{
	Ref<Pipeline> Pipeline::Create(const PipelineSpecification& specifications)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:	NB_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::Vulkan: return CreateRef<Vulkan_Pipeline>(specifications);
		case RendererAPI::API::OpenGL: return CreateRef<Pipeline>();
		}

		NB_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}
}