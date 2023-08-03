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

		VkBuffer GetBuffer() { return m_Buffers[0]->GetBuffer(); }
		VkDeviceSize GetSize() { return m_Buffers[0]->GetSize(); }
		uint32_t GetBinding() { return m_Binding; }
	private:
		uint32_t m_Binding;
		std::vector<Scope<VulkanBuffer>> m_Buffers;

		friend class Vulkan_RendererAPI;
	};
}