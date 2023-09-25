#include "nbpch.h"
#include "Vulkan_Framebuffer.h"

#include "Nebula/Core/Application.h"
#include "Nebula/Renderer/Render_Command.h"

#include "VulkanAPI.h"
#include "Vulkan_Context.h"
#include "Vulkan_RenderPass.h"

namespace Nebula {

	static const uint32_t s_MaxFrameBufferSize = 8192;
	Vulkan_FrameBuffer* Vulkan_FrameBuffer::s_BindedInstance = nullptr;

	namespace Utils 
	{
		bool IsDepthFormat(ImageFormat format) {
			switch (format)
			{
			case ImageFormat::DEPTH24STENCIL8: return true;
			}

			return false;
		}

		VkFormat NebulaToVKImageFormat(ImageFormat format);

		static void CopyImageToBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
		{
			VkBufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;

			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { width, height, 1 };

			vkCmdCopyImageToBuffer(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &region);
		}
	}

	Vulkan_FrameBuffer::Vulkan_FrameBuffer(const FrameBufferSpecification& specifications) 
		: m_Specifications(specifications) 
	{
		for (const auto& spec : m_Specifications.Attachments) 
		{
			if (!Utils::IsDepthFormat(spec.TextureFormat))
				m_ColourAttachmentSpecs.push_back(spec);
			else
				m_DepthAttachmentSpec = spec;
		}

		CreateRenderPass();

		Invalidate();
		s_BindedInstance = this;
	}

	Vulkan_FrameBuffer::~Vulkan_FrameBuffer() 
	{
		VulkanAPI::SubmitResource([framebuffers = m_Framebuffer]()
		{
			for (auto& framebuffer : framebuffers)
				vkDestroyFramebuffer(VulkanAPI::GetDevice(), framebuffer, nullptr);
		});
	}

	void Vulkan_FrameBuffer::CreateRenderPass()
	{
		if (m_Specifications.RenderPass)
			return;

		RenderPassSpecification spec;
		spec.Attachments = m_Specifications.Attachments;
		spec.ClearOnLoad = false;
		
		if (!m_Specifications.DebugName.empty())
			spec.DebugName = m_Specifications.DebugName + "-RenderPass";
		
		m_Specifications.RenderPass = RenderPass::Create(spec);
	}

	void Vulkan_FrameBuffer::Invalidate() 
	{
		if (!m_Framebuffer.empty()) 
		{
			VulkanAPI::SubmitResource([framebuffers = m_Framebuffer]()
			{
				for (auto& framebuffer : framebuffers)
					vkDestroyFramebuffer(VulkanAPI::GetDevice(), framebuffer, nullptr);
			});
			
			m_ColourAttachments.clear();
			m_DepthAttachment = nullptr;
		}

		Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();
		m_ColourAttachments.resize(m_ColourAttachmentSpecs.size());

		for (uint32_t i = 0; i < m_ColourAttachments.size(); i++)
		{
			VkFormat format = Utils::NebulaToVKImageFormat(m_ColourAttachmentSpecs[i].TextureFormat);

			if (m_Specifications.SwapChainTarget && format == context->GetImageFormat())
			{
				m_ColourAttachments[i] = context->m_ImageArray;
				continue;
			}
			
			ImageSpecification imageSpec;
			imageSpec.Format = m_ColourAttachmentSpecs[i].TextureFormat;
			imageSpec.Width = m_Specifications.Width;
			imageSpec.Height = m_Specifications.Height;
			imageSpec.Samples = m_Specifications.Samples;
			imageSpec.ShaderUsage = !m_Specifications.SwapChainTarget;
			imageSpec.Usage = ImageUsage::ColourAttachment | ImageUsage::TransferDst | ImageUsage::TransferSrc | ImageUsage::Sampled;

			if (!m_Specifications.DebugName.empty())
				imageSpec.DebugName = m_Specifications.DebugName + "-Colour_Attachment_" + std::to_string(i);

			m_ColourAttachments[i] = Vulkan_Image::CreateImageArray(imageSpec, context->GetImageCount());
		}

		if (m_DepthAttachmentSpec.TextureFormat != ImageFormat::None)
		{
			ImageSpecification imageSpec;
			imageSpec.Format = m_DepthAttachmentSpec.TextureFormat;
			imageSpec.Width = m_Specifications.Width;
			imageSpec.Height = m_Specifications.Height;
			imageSpec.Samples = m_Specifications.Samples;
			imageSpec.ShaderUsage = false;
			imageSpec.Usage = ImageUsage::DepthStencilAttachment | ImageUsage::TransferDst;

			if (!m_Specifications.DebugName.empty())
				imageSpec.DebugName = m_Specifications.DebugName + "-Depth_Attachment";

			m_DepthAttachment = CreateRef<Vulkan_Image>(imageSpec);
		}

		m_Framebuffer.resize(context->GetImageCount());
		for (uint32_t imageIndex = 0; imageIndex < m_Framebuffer.size(); imageIndex++)
		{
			std::vector<VkImageView> attachments(m_ColourAttachments.size());
			for (uint32_t i = 0; i < attachments.size(); i++)
				attachments[i] = m_ColourAttachments[i][imageIndex]->GetVulkanImageView();

			if (m_DepthAttachmentSpec.TextureFormat != ImageFormat::None)
				attachments.push_back(m_DepthAttachment->GetVulkanImageView());

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = (VkRenderPass)m_Specifications.RenderPass->GetRenderPass();
			framebufferInfo.attachmentCount = (uint32_t)attachments.size();
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = m_Specifications.Width;
			framebufferInfo.height = m_Specifications.Height;
			framebufferInfo.layers = 1;

			VkResult result = vkCreateFramebuffer(VulkanAPI::GetDevice(), &framebufferInfo, nullptr, &m_Framebuffer[imageIndex]);
			NB_ASSERT(result == VK_SUCCESS, "Failed to create framebuffer");

			VulkanAPI::AttachDebugNameToObject(VK_OBJECT_TYPE_FRAMEBUFFER, (uint64_t)m_Framebuffer[imageIndex], m_Specifications.DebugName);
		}

		m_StagingBuffer = CreateScope<VulkanBuffer>(m_Specifications.Width * m_Specifications.Height, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	}

	void Vulkan_FrameBuffer::Bind() 
	{
		s_BindedInstance = this;
		
		// Resize if swaphchain was recreated but no resize
		Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();
		if (m_Specifications.SwapChainTarget && context->m_RecreateSwapChain)
			Invalidate();
	}


	void Vulkan_FrameBuffer::BeginClear()
	{
		m_CommandBuffer = VulkanAPI::BeginSingleUseCommand();
	}
	
	void Vulkan_FrameBuffer::EndClear()
	{
		if (!m_CommandBuffer)
			return;
		
		VulkanAPI::EndSingleUseCommand(m_CommandBuffer);
		m_CommandBuffer = nullptr;
	}
	
	void Vulkan_FrameBuffer::PrepareImages()
	{
		RenderPassSpecification& spec = m_Specifications.RenderPass->GetRenderPassSpecifications();
		if (spec.ClearOnLoad && spec.SingleWrite)
			return; // Image Layout is not important
		
		Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();
		for (auto& attachment : m_ColourAttachments)
		{
			Ref<Vulkan_Image>& image = attachment[context->GetImageIndex()];

			if (m_Specifications.SwapChainTarget && image->GetFormat() == VK_FORMAT_UNDEFINED) // Assume it's from swapchain
				VulkanAPI::TransitionImageLayout(image->GetVulkanImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VulkanAPI::GetCommandBuffer());
				//image->ImageLayout = image->ImageLayout != VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : image->ImageLayout;
		}
	}

	void Vulkan_FrameBuffer::Unbind()
	{
		s_BindedInstance = nullptr;
		EndClear();
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

		Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();
		Ref<Vulkan_Image> image = m_ColourAttachments[attachmentIndex][context->GetImageIndex()];
		
		VkCommandBuffer commandBuffer = VulkanAPI::BeginSingleUseCommand();
		VulkanAPI::TransitionImageLayout(image->GetVulkanImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, commandBuffer);
		Utils::CopyImageToBuffer(commandBuffer, m_StagingBuffer->GetBuffer(), image->GetVulkanImage(), m_Specifications.Width, m_Specifications.Height);
		VulkanAPI::TransitionImageLayout(image->GetVulkanImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, commandBuffer);
		VulkanAPI::EndSingleUseCommand(commandBuffer);

		unsigned int index = x + y * m_Specifications.Width;
		int8_t* data = (int8_t*)m_StagingBuffer->GetMemory();
		int8_t pixel = data[index];
		return pixel;
	}

	void Vulkan_FrameBuffer::ClearAttachment(uint32_t attachmentIndex, VkClearColorValue clearValue)
	{
		NB_ASSERT(attachmentIndex < m_ColourAttachments.size());
		
		Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();
		auto& image = m_ColourAttachments[attachmentIndex][context->GetImageIndex()];// ->GetVulkanImage();

		VkImageSubresourceRange subResourceRange = {};
		subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subResourceRange.baseMipLevel = 0;
		subResourceRange.levelCount = 1;
		subResourceRange.baseArrayLayer = 0;
		subResourceRange.layerCount = 1;

		VkCommandBuffer commandBuffer;
		if (VulkanAPI::IsRecording())
			commandBuffer = VulkanAPI::GetCommandBuffer();
		else
			commandBuffer = m_CommandBuffer ? m_CommandBuffer : VulkanAPI::BeginSingleUseCommand();

		VulkanAPI::TransitionImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, commandBuffer);
		vkCmdClearColorImage(commandBuffer, image->GetVulkanImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue, 1, &subResourceRange);
		VulkanAPI::TransitionImageLayout(image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, commandBuffer);
		
		if (!m_CommandBuffer && !VulkanAPI::IsRecording())
			VulkanAPI::EndSingleUseCommand(commandBuffer);
	}

	void Vulkan_FrameBuffer::ClearAttachment(uint32_t attachmentIndex, int value) 
	{
		VkClearColorValue clearValue = { (float)value };
		ClearAttachment(attachmentIndex, clearValue);
	}
	
	void Vulkan_FrameBuffer::ClearAttachment(uint32_t attachmentIndex, const glm::vec4& value) 
	{
		ClearAttachment(attachmentIndex, VkClearColorValue{ value.r, value.g, value.b, value.a });
	}
	
	void Vulkan_FrameBuffer::ClearDepthAttachment(int value) 
	{
		if (!m_DepthAttachment)
			return;

		Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();
		auto& image = m_DepthAttachment->GetVulkanImage();
		VkImageAspectFlags aspectFlags = m_DepthAttachment->GetAspectFlags();

		VkClearDepthStencilValue clearValue = { 1.0f, (uint32_t)value };

		VkImageSubresourceRange subResourceRange = {};
		subResourceRange.aspectMask = aspectFlags;
		subResourceRange.baseMipLevel = 0;
		subResourceRange.levelCount = 1;
		subResourceRange.baseArrayLayer = 0;
		subResourceRange.layerCount = 1;

		VkCommandBuffer commandBuffer;
		if (VulkanAPI::IsRecording())
			commandBuffer = VulkanAPI::GetCommandBuffer();
		else
			commandBuffer = m_CommandBuffer ? m_CommandBuffer : VulkanAPI::BeginSingleUseCommand();

		VulkanAPI::TransitionImageLayout(image, aspectFlags, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, commandBuffer);
		vkCmdClearDepthStencilImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue, 1, &subResourceRange);
		VulkanAPI::TransitionImageLayout(image, aspectFlags, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, commandBuffer);
		
		if (!m_CommandBuffer && !VulkanAPI::IsRecording())
			VulkanAPI::EndSingleUseCommand(commandBuffer);
	}

	Ref<Image2D> Vulkan_FrameBuffer::GetColourAttachmentImage(uint32_t index) const
	{
		Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();
		return m_ColourAttachments[index][context->GetImageIndex()];
	}

	VkFramebuffer Vulkan_FrameBuffer::GetFrameBuffer()
	{
		Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();
		return m_Framebuffer[context->GetImageIndex()];
	}
}