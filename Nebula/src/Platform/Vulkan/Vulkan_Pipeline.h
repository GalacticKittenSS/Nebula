#pragma once

#include "Nebula/Renderer/Pipeline.h"

#include "VulkanAPI.h"

namespace Nebula
{
	class Vulkan_Pipeline : public Pipeline
	{
	public:
		Vulkan_Pipeline(const PipelineSpecification& specification);
		~Vulkan_Pipeline();

		void Bind() const override;
		void BindDescriptorSet(uint32_t descriptorSet) const override;
		void BindDescriptorSet(uint32_t descriptorSet, Ref<UniformBuffer> uniformBuffer) const override;
	private:
		PipelineSpecification m_Specification;
		VkPipeline m_Pipeline;
		VkPipelineLayout m_Layout;
	};
}