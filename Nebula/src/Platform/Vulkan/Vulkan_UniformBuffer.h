#pragma once

#include "Nebula/Renderer/UniformBuffer.h"

#include <vulkan/vulkan.h>
#include "VulkanAPI.h"

#include <map>

namespace Nebula {

	class Vulkan_UniformBuffer : public UniformBuffer
	{
	public:
		Vulkan_UniformBuffer(uint32_t size, uint32_t binding);
		virtual ~Vulkan_UniformBuffer();

		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
	private:
		std::vector<Scope<VulkanBuffer>> m_Buffers;
		static std::map<uint32_t, Vulkan_UniformBuffer*> s_BindedInstances;

		friend class Vulkan_RendererAPI;
	};
}