#pragma once

#include "Nebula/Renderer/RenderPass.h"

extern "C"
{
	typedef struct VkRenderPass_T* VkRenderPass;
}

namespace Nebula
{
	class Vulkan_RenderPass : public RenderPass
	{
	public:
		Vulkan_RenderPass(const RenderPassSpecifications& attachments);
		~Vulkan_RenderPass();

		void Bind() override;
		void Unbind() override;

		uint64_t GetRenderPass() const override { return (uint64_t)m_RenderPass; }
		
		static VkRenderPass GetVulkanRenderPass() { NB_ASSERT(s_ActiveInstance); return (VkRenderPass)s_ActiveInstance->GetRenderPass(); }
		static const Vulkan_RenderPass* GetActiveInstance() { return s_ActiveInstance; }
	private:
		RenderPassSpecifications m_Specification;
		VkRenderPass m_RenderPass = nullptr;

		static const Vulkan_RenderPass* s_ActiveInstance;
	};
}