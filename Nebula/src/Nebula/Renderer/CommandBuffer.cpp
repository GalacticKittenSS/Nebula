#include "nbpch.h"
#include "CommandBuffer.h"

#include "Renderer_API.h"
#include <Platform/Vulkan/Vulkan_CommandBuffer.h>

namespace Nebula
{
	Ref<CommandBuffer> CommandBuffer::Create()
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan: return CreateRef<Vulkan_CommandBuffer>();
		}

		return nullptr;
	}
}