#pragma once

#include "Nebula/Renderer/CommandBuffer.h"

#include <vulkan/vulkan.h>

namespace Nebula
{
	class Vulkan_CommandBuffer : public CommandBuffer
	{
	public:
		Vulkan_CommandBuffer();
		~Vulkan_CommandBuffer();

		void BeginRecording() override;
		void EndRecording() override;

		bool IsRecording() override { return m_Recording; }
		uint64_t GetCommandBuffer() const override { return (uint64_t)m_CommandBuffer; }

		static bool IsBound() { return s_Instance; }
		static const VkCommandBuffer& GetVulkanBuffer() { return *s_Instance; }
	private:
		void Submit();
	private:
		VkCommandBuffer m_CommandBuffer;
		VkFence m_Fence;

		bool m_Recording = false;

		static VkCommandBuffer* s_Instance;
	};
}