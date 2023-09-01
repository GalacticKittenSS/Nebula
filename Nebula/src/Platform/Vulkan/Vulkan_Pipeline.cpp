#include "nbpch.h"
#include "Vulkan_Pipeline.h"

#include "Vulkan_Shader.h"
#include "Vulkan_UniformBuffer.h"

namespace Nebula
{
	namespace Utils
	{
		VkPrimitiveTopology NBShapeToPrimitiveTopology(PipelineShape shape)
		{
			switch (shape)
			{
			case Nebula::PipelineShape::Triangles: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case Nebula::PipelineShape::Lines: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			}
			
			NB_ASSERT(false, "Unknown Shape Given");
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}

		VkPolygonMode NBShapeToPolygonMode(PipelineShape shape)
		{
			switch (shape)
			{
			case Nebula::PipelineShape::Triangles: return VK_POLYGON_MODE_FILL;
			case Nebula::PipelineShape::Lines: return VK_POLYGON_MODE_LINE;
			}
			
			NB_ASSERT(false, "Unknown Shape Given");
			return VK_POLYGON_MODE_FILL;
		}
	}

	Vulkan_Pipeline::Vulkan_Pipeline(const PipelineSpecification& specification)
		: m_Specification(specification)
	{
		Ref<Vulkan_Shader> shader = std::static_pointer_cast<Vulkan_Shader>(m_Specification.Shader);

		// Pipeline Layout
		{
			const std::vector<VkDescriptorSetLayout>& descriptorSets = shader->GetVulkanDescriptorSetLayouts();

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = (uint32_t)descriptorSets.size();
			pipelineLayoutInfo.pSetLayouts = descriptorSets.data();

			VkResult result = vkCreatePipelineLayout(VulkanAPI::GetDevice(), &pipelineLayoutInfo, nullptr, &m_Layout);
			NB_ASSERT(result == VK_SUCCESS, "Failed to create pipeline layout");
		}

		uint32_t stride;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		shader->GetVulkanVertexInputInfo(attributeDescriptions, stride);
		
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = stride;
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = attributeDescriptions.size() > 0 ? 1 : 0;
		vertexInputInfo.pVertexBindingDescriptions = attributeDescriptions.size() > 0 ? &bindingDescription : nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = Utils::NBShapeToPrimitiveTopology(m_Specification.Shape);
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = Utils::NBShapeToPolygonMode(m_Specification.Shape);
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		size_t outputCount = m_Specification.Shader->GetFragmentOutputCount();
		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(outputCount);

		for (uint32_t i = 0; i < outputCount; i++)
		{
			colorBlendAttachments[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachments[i].blendEnable = VK_FALSE;
		}

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = (uint32_t)colorBlendAttachments.size();
		colorBlending.pAttachments = colorBlendAttachments.data();
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = m_Specification.DepthTestEnable ? VK_TRUE : VK_FALSE;
		depthStencil.depthWriteEnable = m_Specification.DepthTestEnable ? VK_TRUE : VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS; // Specification?
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional
		depthStencil.front = {}; // Optional
		depthStencil.back = {}; // Optional

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_CULL_MODE,
			VK_DYNAMIC_STATE_LINE_WIDTH
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shader->GetVulkanShaderStages().data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = m_Layout;
		pipelineInfo.renderPass = (VkRenderPass)m_Specification.RenderPass->GetRenderPass();
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		VkResult result = vkCreateGraphicsPipelines(VulkanAPI::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline);
		NB_ASSERT(result == VK_SUCCESS, "Failed to create graphics pipeline!");
	}

	Vulkan_Pipeline::~Vulkan_Pipeline()
	{
		VulkanAPI::SubmitResource([pipeline = m_Pipeline, layout = m_Layout]()
		{
			vkDestroyPipeline(VulkanAPI::GetDevice(), pipeline, nullptr);
			vkDestroyPipelineLayout(VulkanAPI::GetDevice(), layout, nullptr);
		});
	}

	void Vulkan_Pipeline::Bind() const 
	{
		vkCmdBindPipeline(VulkanAPI::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
	}

	void Vulkan_Pipeline::BindDescriptorSet(uint32_t descriptorSet, Ref<UniformBuffer> uniformBuffer) const
	{
		Ref<Vulkan_UniformBuffer> vulkanBuffer = std::static_pointer_cast<Vulkan_UniformBuffer>(uniformBuffer);
		vkCmdBindDescriptorSets(VulkanAPI::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Layout,
			descriptorSet, 1, &vulkanBuffer->GetDescriptorSet(), 0, nullptr);
	}

	void Vulkan_Pipeline::BindDescriptorSet(uint32_t descriptorSet) const
	{
		Ref<Vulkan_Shader> vulkanShader = std::static_pointer_cast<Vulkan_Shader>(m_Specification.Shader);
		const auto& descriptorSets = vulkanShader->GetVulkanDescriptorSets();

		if (descriptorSet == (uint32_t)-1)
			vkCmdBindDescriptorSets(VulkanAPI::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Layout,
				0, (uint32_t)descriptorSets.size(), descriptorSets.data(), 0, nullptr);
		else
			vkCmdBindDescriptorSets(VulkanAPI::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Layout,
				descriptorSet, 1, &descriptorSets[descriptorSet], 0, nullptr);
	}
}