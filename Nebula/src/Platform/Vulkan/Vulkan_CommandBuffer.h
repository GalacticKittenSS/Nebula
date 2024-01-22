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
		void Submit() override;

		bool IsRecording() override { return m_Recording; }

		const VkCommandBuffer& GetCommandBuffer() const { return m_CommandBuffer; }
		static const Vulkan_CommandBuffer* GetInstance() { return s_Instance; }
	private:
		VkCommandBuffer m_CommandBuffer;
		VkFence m_Fence;

		bool m_Recording = false;

		static Vulkan_CommandBuffer* s_Instance;
	};
}