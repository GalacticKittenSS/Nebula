#include "nbpch.h"
#include "Vulkan_Framebuffer.h"

#include "Nebula/Renderer/Render_Command.h"
#include "Nebula/Core/Application.h"
#include "VulkanAPI.h"
#include "Vulkan_Context.h"

namespace Nebula {

	static const uint32_t s_MaxFrameBufferSize = 8192;
	Vulkan_FrameBuffer* Vulkan_FrameBuffer::s_BindedInstance = nullptr;

	namespace Utils {
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
			case Nebula::FramebufferTextureFormat::RED_INT: return VK_FORMAT_R8_UINT;
			}

			NB_ASSERT(false);
			return VK_FORMAT_R8G8B8_UINT;
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
	}

	Vulkan_FrameBuffer::~Vulkan_FrameBuffer() 
	{
		for (auto& framebuffer : m_Framebuffer)
		{
			vkDestroyFramebuffer(VulkanAPI::GetDevice(), framebuffer, nullptr);
		}
		
		vkDestroyRenderPass(VulkanAPI::GetDevice(), m_RenderPass, nullptr);
		vkDestroyRenderPass(VulkanAPI::GetDevice(), m_RenderPass, nullptr);
	}

	void Vulkan_FrameBuffer::Invalidate() {
		if (!m_Framebuffer.empty()) 
		{
			for (auto& framebuffer : m_Framebuffer)
			{
				vkDestroyFramebuffer(VulkanAPI::GetDevice(), framebuffer, nullptr);
			}
			
			vkDestroyRenderPass(VulkanAPI::GetDevice(), m_RenderPass, nullptr);

			for (auto& imageView : m_ColourAttachments) 
			{
				vkDestroyImageView(VulkanAPI::GetDevice(), imageView, nullptr);
			}
			vkDestroyImageView(VulkanAPI::GetDevice(), m_DepthAttachment, nullptr);
			
			m_ColourAttachments.clear();
			m_DepthAttachment = VK_NULL_HANDLE;
		}

		/*bool multiSample = m_Specifications.samples > 1;

		if (m_ColourAttachmentSpecs.size())
		{
			m_ColourAttachments.resize(m_ColourAttachmentSpecs.size());
			Utils::CreateTextures(multiSample, m_ColourAttachments.data(), (GLsizei)m_ColourAttachments.size());

			for (size_t i = 0; i < m_ColourAttachments.size(); i++) 
			{
				Utils::BindTexture(multiSample, m_ColourAttachments[i]);
				switch (m_ColourAttachmentSpecs[i].TextureFormat) {
				case FramebufferTextureFormat::RGBA8:
					Utils::AttachColourTexture(m_ColourAttachments[i], m_Specifications.samples, GL_RGBA8, GL_RGBA, m_Specifications.Width, m_Specifications.Height, (int)i);
					break;
				case FramebufferTextureFormat::RED_INT:
					Utils::AttachColourTexture(m_ColourAttachments[i], m_Specifications.samples, GL_R32I, GL_RED_INTEGER, m_Specifications.Width, m_Specifications.Height, (int)i);
					break;
				}
			}
		}

		if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None) 
		{
			Utils::CreateTextures(multiSample, &m_DepthAttachment, 1);
			Utils::BindTexture(multiSample, m_DepthAttachment);
			
			switch (m_DepthAttachmentSpec.TextureFormat)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8:
				Utils::AttachDepthTexture(m_DepthAttachment, m_Specifications.samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specifications.Width, m_Specifications.Height);
				break;
			}
		}*/

		std::vector<VkAttachmentDescription> attachmentDesc;
		std::vector<VkAttachmentReference> attachmentRef;

		if (m_ColourAttachmentSpecs.size())
		{
			m_ColourAttachments.resize(m_ColourAttachmentSpecs.size());
			attachmentDesc.resize(m_ColourAttachmentSpecs.size());
			attachmentRef.resize(m_ColourAttachmentSpecs.size());

			for (size_t i = 0; i < m_ColourAttachmentSpecs.size(); i++)
			{
				VkFormat format = Utils::NebulaFBFormattoVulkan(m_ColourAttachmentSpecs[i].TextureFormat);

				VkAttachmentDescription colourAttachment;
				colourAttachment.format = format;
				colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
				colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				colourAttachment.flags = 0;
				attachmentDesc[i] = colourAttachment;

				VkAttachmentReference reference;
				reference.attachment = i;
				reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attachmentRef[i] = reference;
			}
		}

		/*VkAttachmentReference depthReference;

		if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None)
		{
			VkFormat format = VK_FORMAT_D16_UNORM;

			VkAttachmentDescription depthAttachment;
			depthAttachment.format = format;
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			depthAttachment.flags = 0;
			attachmentDesc.push_back(depthAttachment);

			depthReference.attachment = m_ColourAttachments.size();
			depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		}*/

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = (uint32_t)attachmentRef.size();
		subpass.pColorAttachments = attachmentRef.data();
		//subpass.pDepthStencilAttachment = &depthReference;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

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


		Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();
		
		m_Framebuffer.resize(context->m_Images.size());
		for (uint32_t imageIndex = 0; imageIndex < context->m_Images.size(); imageIndex++)
		{
			std::vector<VkImageView> attachments;
			
			if (m_ColourAttachmentSpecs.size())
			{
				m_ColourAttachments.resize(m_ColourAttachmentSpecs.size());
				attachments.resize(m_ColourAttachmentSpecs.size());
			
				for (size_t i = 0; i < m_ColourAttachmentSpecs.size(); i++)
				{
					VkFormat format = Utils::NebulaFBFormattoVulkan(m_ColourAttachmentSpecs[i].TextureFormat);

					/*VkImageCreateInfo imageInfo{};
					imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
					imageInfo.imageType = VK_IMAGE_TYPE_2D;
					imageInfo.extent.width = m_Specifications.Width;
					imageInfo.extent.height = m_Specifications.Height;
					imageInfo.extent.depth = 1;
					imageInfo.mipLevels = 1;
					imageInfo.arrayLayers = 1;
					imageInfo.format = format;
					imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
					imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
					imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
					imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
					imageInfo.flags = 0;

					VkResult result = vkCreateImage((VkDevice)RenderCommand::GetDevice(), &imageInfo, nullptr, &images[i]);
					NB_ASSERT(result == VK_SUCCESS, "Failed to create image!");

					VkMemoryRequirements memRequirements;
					vkGetImageMemoryRequirements((VkDevice)RenderCommand::GetDevice(), images[i], &memRequirements);

					VkMemoryAllocateInfo allocInfo;
					allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
					allocInfo.allocationSize = memRequirements.size;
					allocInfo.memoryTypeIndex = find(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

					result = vkAllocateMemory((VkDevice)RenderCommand::GetDevice(), &allocInfo, nullptr, &imageMemory);
					NB_ASSERT(result == VK_SUCCESS, "Failed to allocate image memory!");

					vkBindImageMemory((VkDevice)RenderCommand::GetDevice(), images[i], imageMemory, 0);
					*/
					VkImageViewCreateInfo createInfo{};
					createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
					createInfo.image = context->m_Images[imageIndex];
					createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
					createInfo.format = format;
					createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
					createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
					createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
					createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
					createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					createInfo.subresourceRange.baseMipLevel = 0;
					createInfo.subresourceRange.levelCount = 1;
					createInfo.subresourceRange.baseArrayLayer = 0;
					createInfo.subresourceRange.layerCount = 1;

					result = vkCreateImageView(VulkanAPI::GetDevice(), &createInfo, nullptr, &m_ColourAttachments[i]);
					NB_ASSERT(result == VK_SUCCESS, "Failed to create image view!");

					attachments[i] = m_ColourAttachments[i];
				}
			}

			/*if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None)
			{
				VkFormat format = VK_FORMAT_D16_UNORM;

				/*VkImageCreateInfo imageInfo{};
				imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				imageInfo.imageType = VK_IMAGE_TYPE_2D;
				imageInfo.extent.width = m_Specifications.Width;
				imageInfo.extent.height = m_Specifications.Height;
				imageInfo.extent.depth = 1;
				imageInfo.mipLevels = 1;
				imageInfo.arrayLayers = 1;
				imageInfo.format = format;
				imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
				imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
				imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
				imageInfo.flags = 0;

				VkImage depthImage;
				VkResult result = vkCreateImage((VkDevice)RenderCommand::GetDevice(), &imageInfo, nullptr, &depthImage);
				NB_ASSERT(result == VK_SUCCESS, "Failed to create image!");

				VkImageViewCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				createInfo.image = context->m_Images[imageIndex];
				createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				createInfo.format = format;
				createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				createInfo.subresourceRange.baseMipLevel = 0;
				createInfo.subresourceRange.levelCount = 1;
				createInfo.subresourceRange.baseArrayLayer = 0;
				createInfo.subresourceRange.layerCount = 1;

				result = vkCreateImageView((VkDevice)RenderCommand::GetDevice(), &createInfo, nullptr, &m_DepthAttachment);
				NB_ASSERT(result == VK_SUCCESS, "Failed to create image view!");

				attachments.push_back(m_DepthAttachment);
			}*/

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
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)m_Specifications.Width;
		viewport.height = (float)m_Specifications.Height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		//vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkExtent2D extent;
		extent.width = m_Specifications.Width;
		extent.height = m_Specifications.Height;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;
		//vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		s_BindedInstance = this;
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

	void Vulkan_FrameBuffer::ClearAttachment(uint32_t attachmentIndex, int value) 
	{
		NB_ASSERT(attachmentIndex < m_ColourAttachments.size(), "");
	}
}