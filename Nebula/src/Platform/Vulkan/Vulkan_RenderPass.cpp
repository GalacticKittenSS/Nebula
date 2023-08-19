#include "nbpch.h"
#include "Vulkan_RenderPass.h"

#include "Nebula/Core/Application.h"

#include "VulkanAPI.h"
#include "Vulkan_Context.h"
#include "Vulkan_Framebuffer.h"

namespace Nebula
{
	namespace Utils
	{
		bool IsDepthFormat(AttachmentTextureFormat format);
		VkFormat NebulaFBFormattoVulkan(AttachmentTextureFormat format);
		VkFormat FindDepthFormat();
	}

	const Vulkan_RenderPass* Vulkan_RenderPass::s_ActiveInstance = nullptr;
	
	Vulkan_RenderPass::Vulkan_RenderPass(const RenderPassSpecification& specification)
		: m_Specification(specification)
	{
		AttachmentTextureSpecification depthAttachment;
		std::vector<AttachmentTextureSpecification> colourAttachments;
		for (auto& spec : m_Specification.Attachments)
		{
			if (Utils::IsDepthFormat(spec.TextureFormat))
			{
				depthAttachment = spec;
			}
			else
			{
				colourAttachments.push_back(spec);
			}
		}

		std::vector<VkAttachmentDescription> attachmentDesc(colourAttachments.size());
		std::vector<VkAttachmentReference> attachmentRef(colourAttachments.size());

		for (uint32_t i = 0; i < colourAttachments.size(); i++)
		{
			VkFormat format = Utils::NebulaFBFormattoVulkan(colourAttachments[i].TextureFormat);

			VkAttachmentDescription colourAttachment;
			colourAttachment.format = format;
			colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colourAttachment.loadOp = m_Specification.ClearOnLoad ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
			colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colourAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colourAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colourAttachment.flags = 0;
			attachmentDesc[i] = colourAttachment;

			VkAttachmentReference reference;
			reference.attachment = i;
			reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachmentRef[i] = reference;
		}

		VkAttachmentReference* depthReference = nullptr;

		if (depthAttachment.TextureFormat != AttachmentTextureFormat::None)
		{
			VkFormat format = Utils::FindDepthFormat();

			VkAttachmentDescription depthAttachment;
			depthAttachment.format = format;
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthAttachment.flags = 0;
			attachmentDesc.push_back(depthAttachment);

			depthReference = new VkAttachmentReference();
			depthReference->attachment = (uint32_t)colourAttachments.size();
			depthReference->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = (uint32_t)attachmentRef.size();
		subpass.pColorAttachments = attachmentRef.data();
		subpass.pDepthStencilAttachment = depthReference;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		
		if (m_Specification.ShaderOnly)
		{
			dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		}
		else
		{
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
		}

		if (depthAttachment.TextureFormat != AttachmentTextureFormat::None)
		{
			dependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = (uint32_t)attachmentDesc.size();
		renderPassInfo.pAttachments = attachmentDesc.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(VulkanAPI::GetDevice(), &renderPassInfo, nullptr, &m_RenderPass);
		NB_ASSERT(result == VK_SUCCESS, "Failed to create render pass!");

		delete depthReference;
	}

	Vulkan_RenderPass::~Vulkan_RenderPass()
	{
		VulkanAPI::SubmitResource([renderPass = m_RenderPass]()
		{
			vkDestroyRenderPass(VulkanAPI::GetDevice(), renderPass, nullptr);
		});
	}

	void Vulkan_RenderPass::Bind()
	{
		s_ActiveInstance = this;
		Vulkan_FrameBuffer* framebuffer = Vulkan_FrameBuffer::GetActiveInstance();

		if (!VulkanAPI::IsRecording() || !framebuffer)
			return;

		Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();
		uint32_t imageIndex = context->m_ImageIndex;
		if (imageIndex == (uint32_t)-1)
			return;

		FrameBufferSpecification spec = framebuffer->GetFrameBufferSpecifications();

		VkExtent2D extent;
		extent.width = spec.Width;
		extent.height = spec.Height;

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_RenderPass;
		renderPassInfo.framebuffer = framebuffer->m_Framebuffer[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = extent;

		std::vector<VkClearValue> clearValues(m_Specification.Attachments.size());
		renderPassInfo.clearValueCount = (uint32_t)clearValues.size();
		renderPassInfo.pClearValues = clearValues.data();

		VkCommandBuffer commandBuffer = VulkanAPI::GetCommandBuffer();
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)extent.width;
		viewport.height = (float)extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		if (spec.SwapChainTarget)
		{
			viewport.y = (float)extent.height;
			viewport.height = -(float)extent.height;
		}

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void Vulkan_RenderPass::Unbind()
	{
		s_ActiveInstance = nullptr;

		if (VulkanAPI::IsRecording())
		{
			vkCmdEndRenderPass(VulkanAPI::GetCommandBuffer());
		}
	}
}