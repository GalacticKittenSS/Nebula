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
		Vulkan_RenderPass(const RenderPassSpecification& attachments);
		~Vulkan_RenderPass();

		void Bind() override;
		void Unbind() override;

		uint64_t GetRenderPass() const override { return (uint64_t)m_RenderPass; }
		
		RenderPassSpecification& GetRenderPassSpecifications() override { return m_Specification; }
		const RenderPassSpecification& GetRenderPassSpecifications() const override { return m_Specification; }
	private:
		RenderPassSpecification m_Specification;
		VkRenderPass m_RenderPass = nullptr;
	};
}