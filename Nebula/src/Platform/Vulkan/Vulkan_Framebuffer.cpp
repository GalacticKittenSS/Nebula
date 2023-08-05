#include "nbpch.h"
#include "Vulkan_Framebuffer.h"

#include "Nebula/Renderer/Render_Command.h"
#include "Nebula/Core/Application.h"
#include "VulkanAPI.h"
#include "Vulkan_Context.h"
#include "VulkanAPI.h"

namespace Nebula {

	static const uint32_t s_MaxFrameBufferSize = 8192;
	Vulkan_FrameBuffer* Vulkan_FrameBuffer::s_BindedInstance = nullptr;

	namespace Utils 
	{
		static bool IsDepthFormat(FramebufferTextureFormat format) {
			switch (format)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8: return true;
			}

			return false;
		}

		static VkFormat NebulaFBFormattoVulkan(FramebufferTextureFormat format) {
			switch (format)
			{
			case Nebula::FramebufferTextureFormat::RGBA8:	return VK_FORMAT_B8G8R8A8_SRGB;
			case Nebula::FramebufferTextureFormat::RED_INT: return VK_FORMAT_R8_SINT;
			}

			NB_ASSERT(false);
			return VK_FORMAT_R8G8B8_UINT;
		}

		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
		{
			for (VkFormat format : candidates)
			{
				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(VulkanAPI::GetPhysicalDevice(), format, &props);

				if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
					return format;
				else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
					return format;
			}

			NB_ASSERT(false, "Failed to find supported format!")
		}

		VkFormat FindDepthFormat()
		{
			return FindSupportedFormat(
				{ VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT },
				VK_IMAGE_TILING_OPTIMAL,
				VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
			);
		}

		bool HasStencilComponent(VkFormat format)
		{
			return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
		}

		VkImageAspectFlags GetDepthAspectFlags(VkFormat format)
		{
			VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (Utils::HasStencilComponent(format))
				aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;

			return aspectFlags;
		}
	}

	Vulkan_FrameBuffer::Vulkan_FrameBuffer(const FrameBufferSpecification& specifications) 
		: m_Specifications(specifications) 
	{
		for (auto spec : m_Specifications.Attachments.Attachments) 
		{
			if (!Utils::IsDepthFormat(spec.TextureFormat))
				m_ColourAttachmentSpecs.push_back(spec);
			else
				m_DepthAttachmentSpec = spec;
		}

		Invalidate();
		s_BindedInstance = this;
	}

	Vulkan_FrameBuffer::~Vulkan_FrameBuffer() 
	{
		for (auto& framebuffer : m_Framebuffer)
		{
			vkDestroyFramebuffer(VulkanAPI::GetDevice(), framebuffer, nullptr);
		}
		
		vkDestroyRenderPass(VulkanAPI::GetDevice(), m_RenderPass, nullptr);
	}

	void Vulkan_FrameBuffer::Invalidate() 
	{
		if (!m_Framebuffer.empty()) 
		{
			vkDeviceWaitIdle(VulkanAPI::GetDevice());

			for (auto& framebuffer : m_Framebuffer)
				vkDestroyFramebuffer(VulkanAPI::GetDevice(), framebuffer, nullptr);
			
			vkDestroyRenderPass(VulkanAPI::GetDevice(), m_RenderPass, nullptr);

			m_ColourAttachments.clear();
			m_DepthAttachment = nullptr;
		}

		std::vector<VkAttachmentDescription> attachmentDesc(m_ColourAttachmentSpecs.size());
		std::vector<VkAttachmentReference> attachmentRef(m_ColourAttachmentSpecs.size());
		m_ColourAttachments.resize(m_ColourAttachmentSpecs.size());
		
		Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();

		for (uint32_t i = 0; i < m_ColourAttachments.size(); i++)
		{
			VkFormat format = Utils::NebulaFBFormattoVulkan(m_ColourAttachmentSpecs[i].TextureFormat);
			
			VkAttachmentDescription colourAttachment;
			colourAttachment.format = format;
			colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colourAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colourAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colourAttachment.flags = 0;

			if (m_Specifications.SwapChainTarget && format == *(VkFormat*)context->GetImageFormat())
			{
				m_ColourAttachments[i] = VulkanImage::CreateImageArray(context->m_Images, context->m_ImageViews);
			}
			else
			{
				m_ColourAttachments[i] = VulkanImage::CreateImageArray(format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
					m_Specifications.samples, m_Specifications.Width, m_Specifications.Height);
			}

			attachmentDesc[i] = colourAttachment;

			VkAttachmentReference reference;
			reference.attachment = i;
			reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachmentRef[i] = reference;
		}

		VkAttachmentReference depthReference;

		if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None)
		{
			VkFormat format = Utils::FindDepthFormat();

			m_DepthAttachment = CreateRef<VulkanImage>(format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 
				Utils::GetDepthAspectFlags(format), m_Specifications.samples, m_Specifications.Width, m_Specifications.Height);

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

			depthReference.attachment = (uint32_t)m_ColourAttachments.size();
			depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = (uint32_t)attachmentRef.size();
		subpass.pColorAttachments = attachmentRef.data();
		subpass.pDepthStencilAttachment = &depthReference;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		
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

		m_Framebuffer.resize(context->GetImageCount());

		for (uint32_t imageIndex = 0; imageIndex < m_Framebuffer.size(); imageIndex++)
		{
			std::vector<VkImageView> attachments(m_ColourAttachments.size());
			for (uint32_t i = 0; i < attachments.size(); i++)
				attachments[i] = m_ColourAttachments[i][imageIndex]->GetImageView();

			if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None)
				attachments.push_back(m_DepthAttachment->GetImageView());

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.attachmentCount = (uint32_t)attachments.size();
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = m_Specifications.Width;
			framebufferInfo.height = m_Specifications.Height;
			framebufferInfo.layers = 1;

			VkResult result = vkCreateFramebuffer(VulkanAPI::GetDevice(), &framebufferInfo, nullptr, &m_Framebuffer[imageIndex]);
			NB_ASSERT(result == VK_SUCCESS, "Failed to create framebuffer");
		}
	}

	void Vulkan_FrameBuffer::Bind() 
	{
		s_BindedInstance = this;
	}

	void Vulkan_FrameBuffer::BeginRenderPass()
	{
		//VulkanAPI::BeginFrame();

		Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();
		uint32_t imageIndex = context->m_ImageIndex;
		if (imageIndex == (uint32_t)-1)
			return;

		VkExtent2D extent;
		extent.width = s_BindedInstance->m_Specifications.Width;
		extent.height = s_BindedInstance->m_Specifications.Height;

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = s_BindedInstance->m_RenderPass;
		renderPassInfo.framebuffer = s_BindedInstance->m_Framebuffer[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = extent;

		uint32_t attachmentCount = s_BindedInstance->m_ColourAttachments.size();
		if (s_BindedInstance->m_DepthAttachment)
			attachmentCount++;

		std::vector<VkClearValue> clearValues(attachmentCount);
		renderPassInfo.clearValueCount = (uint32_t)clearValues.size();
		renderPassInfo.pClearValues = clearValues.data();

		VkCommandBuffer commandBuffer = VulkanAPI::GetCommandBuffer();
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = (float)extent.height;
		viewport.width = (float)extent.width;
		viewport.height = -(float)extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void Vulkan_FrameBuffer::Unbind()
	{
		s_BindedInstance = nullptr;
	}

	void Vulkan_FrameBuffer::Resize(uint32_t width, uint32_t height) 
	{
		if (width == 0 || height == 0 || width > s_MaxFrameBufferSize || height > s_MaxFrameBufferSize) 
		{
			NB_WARN("Attempted to resize framebuffer to ({0}, {1})", width, height);
			return;
		}

		m_Specifications.Width = width;
		m_Specifications.Height = height;

		Invalidate();
	}


	int Vulkan_FrameBuffer::ReadPixel(uint32_t attachmentIndex, int x, int y) 
	{
		NB_ASSERT(attachmentIndex < m_ColourAttachments.size(), "Index is greater than Attachment Size");
		return 0;
	}

	void Vulkan_FrameBuffer::ClearAttachment(uint32_t attachmentIndex, VkClearColorValue clearValue)
	{
		NB_ASSERT(attachmentIndex < m_ColourAttachments.size());

		Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();
		auto& image = m_ColourAttachments[attachmentIndex][context->m_ImageIndex]->GetImage();

		VkCommandBuffer commandBuffer = VulkanAPI::BeginSingleUseCommand();
		
		VkImageSubresourceRange subResourceRange = {};
		subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subResourceRange.baseMipLevel = 0;
		subResourceRange.levelCount = 1;
		subResourceRange.baseArrayLayer = 0;
		subResourceRange.layerCount = 1;

		VulkanAPI::TransitionImageLayout(image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, commandBuffer);
		vkCmdClearColorImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue, 1, &subResourceRange);
		VulkanAPI::TransitionImageLayout(image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, commandBuffer);

		VulkanAPI::EndSingleUseCommand(commandBuffer);
	}

	void Vulkan_FrameBuffer::ClearAttachment(uint32_t attachmentIndex, int value) 
	{
		VkClearColorValue clearValue = { value };
		ClearAttachment(attachmentIndex, clearValue);
	}
	
	void Vulkan_FrameBuffer::ClearAttachment(uint32_t attachmentIndex, const glm::vec4& value) 
	{
		ClearAttachment(attachmentIndex, VkClearColorValue{ value.r, value.g, value.b, value.a });
	}
	
	void Vulkan_FrameBuffer::ClearDepthAttachment(int value) 
	{
		NB_ASSERT(m_DepthAttachment);

		Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();
		auto& image = m_DepthAttachment->GetImage();
		VkImageAspectFlags aspectFlags = m_DepthAttachment->GetAspectFlags();

		VkCommandBuffer commandBuffer = VulkanAPI::BeginSingleUseCommand();
		VkClearDepthStencilValue clearValue = { 1.0f, value };

		VkImageSubresourceRange subResourceRange = {};
		subResourceRange.aspectMask = aspectFlags;
		subResourceRange.baseMipLevel = 0;
		subResourceRange.levelCount = 1;
		subResourceRange.baseArrayLayer = 0;
		subResourceRange.layerCount = 1;

		VulkanAPI::TransitionImageLayout(image, aspectFlags, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, commandBuffer);
		vkCmdClearDepthStencilImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue, 1, &subResourceRange);
		VulkanAPI::TransitionImageLayout(image, aspectFlags, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, commandBuffer);

		VulkanAPI::EndSingleUseCommand(commandBuffer);
	}
}