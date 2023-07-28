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
	}

	void Vulkan_FrameBuffer::Invalidate() 
	{
		if (!m_Framebuffer.empty()) 
		{
			for (auto& framebuffer : m_Framebuffer)
			{
				vkDestroyFramebuffer(VulkanAPI::GetDevice(), framebuffer, nullptr);
			}
			
			vkDestroyRenderPass(VulkanAPI::GetDevice(), m_RenderPass, nullptr);

			m_ColourAttachments.clear();
			m_DepthAttachment = nullptr;
		}

		std::vector<VkAttachmentDescription> attachmentDesc(m_ColourAttachmentSpecs.size());
		std::vector<VkAttachmentReference> attachmentRef(m_ColourAttachmentSpecs.size());
		m_ColourAttachments.resize(m_ColourAttachmentSpecs.size());
		
		for (size_t i = 0; i < m_ColourAttachments.size(); i++)
		{
			VkFormat format = Utils::NebulaFBFormattoVulkan(m_ColourAttachmentSpecs[i].TextureFormat);
			
			m_ColourAttachments[i] = CreateRef<VulkanImage>(format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
				m_Specifications.samples, m_Specifications.Width, m_Specifications.Height);
			
			VkAttachmentDescription colourAttachment;
			colourAttachment.format = format;
			colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colourAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			colourAttachment.flags = 0;
			attachmentDesc[i] = colourAttachment;

			VkAttachmentReference reference;
			reference.attachment = i;
			reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachmentRef[i] = reference;
		}

		VkAttachmentReference depthReference;

		if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None)
		{
			m_DepthAttachment = CreateRef<VulkanImage>(VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT,
				m_Specifications.samples, m_Specifications.Width, m_Specifications.Height);

			VkAttachmentDescription depthAttachment;
			depthAttachment.format = VK_FORMAT_D16_UNORM;
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
		m_Framebuffer.resize(context->GetImageCount());

		for (uint32_t imageIndex = 0; imageIndex < m_Framebuffer.size(); imageIndex++)
		{
			std::vector<VkImageView> attachments(m_ColourAttachments.size());
			for (uint32_t i = 0; i < attachments.size(); i++)
				attachments[i] = m_ColourAttachments[i]->GetImageViews()[imageIndex];

			if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None)
				attachments.push_back(m_DepthAttachment->GetImageViews()[imageIndex]);

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