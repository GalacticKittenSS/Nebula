#include "nbpch.h"
#include "Vulkan_Framebuffer.h"

#include "Nebula/Core/Application.h"

#include "VulkanAPI.h"
#include "Vulkan_Context.h"
#include "Vulkan_RenderPass.h"

#include <backends/imgui_impl_vulkan.h>

namespace Nebula {

	static const uint32_t s_MaxFrameBufferSize = 8192;
	Vulkan_FrameBuffer* Vulkan_FrameBuffer::s_BindedInstance = nullptr;

	namespace Utils 
	{
		bool IsDepthFormat(AttachmentTextureFormat  format) {
			switch (format)
			{
			case AttachmentTextureFormat::DEPTH24STENCIL8: return true;
			}

			return false;
		}

		VkFormat NebulaFBFormattoVulkan(AttachmentTextureFormat  format) {
			switch (format)
			{
			case Nebula::AttachmentTextureFormat::RGBA8:	return VK_FORMAT_B8G8R8A8_UNORM;
			case Nebula::AttachmentTextureFormat::RED_INT: return VK_FORMAT_R8_SINT;
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

			NB_ASSERT(false, "Failed to find supported format!");
			return VK_FORMAT_UNDEFINED;
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
		for (const auto& spec : m_Specifications.Attachments.Attachments) 
		{
			if (!Utils::IsDepthFormat(spec.TextureFormat))
				m_ColourAttachmentSpecs.push_back(spec);
			else
				m_DepthAttachmentSpec = spec;
		}

		if (!m_Specifications.SwapChainTarget)
		{
			VkPhysicalDeviceProperties properties{};
			vkGetPhysicalDeviceProperties(VulkanAPI::GetPhysicalDevice(), &properties);

			VkSamplerCreateInfo samplerInfo{};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.anisotropyEnable = VK_TRUE;
			samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

			VkResult result = vkCreateSampler(VulkanAPI::GetDevice(), &samplerInfo, nullptr, &m_ImGuiSampler);
			NB_ASSERT(result == VK_SUCCESS, "Failed to create texture sampler!");
		}
		
		CreateRenderPass();

		Invalidate();
		s_BindedInstance = this;
	}

	Vulkan_FrameBuffer::~Vulkan_FrameBuffer() 
	{
		for (auto& framebuffer : m_Framebuffer)
		{
			vkDestroyFramebuffer(VulkanAPI::GetDevice(), framebuffer, nullptr);
		}
		
		if (m_ImGuiSampler)
			vkDestroySampler(VulkanAPI::GetDevice(), m_ImGuiSampler, nullptr);

		for (auto& descriptor : m_ImGuiDescriptors)
			ImGui_ImplVulkan_RemoveTexture(descriptor);
	}

	void Vulkan_FrameBuffer::CreateRenderPass()
	{
		if (m_Specifications.RenderPass)
			return;

		RenderPassSpecification spec;
		spec.Attachments = m_Specifications.Attachments.Attachments;
		spec.ClearOnLoad = false;
		spec.ShaderOnly = !m_Specifications.SwapChainTarget;
		m_Specifications.RenderPass = RenderPass::Create(spec);
	}

	void Vulkan_FrameBuffer::Invalidate() 
	{
		if (!m_Framebuffer.empty()) 
		{
			for (auto& framebuffer : m_Framebuffer)
				vkDestroyFramebuffer(VulkanAPI::GetDevice(), framebuffer, nullptr);

			for (auto& descriptor : m_ImGuiDescriptors)
				ImGui_ImplVulkan_RemoveTexture(descriptor);
			
			m_ImGuiDescriptors.clear();
			m_ColourAttachments.clear();
			m_DepthAttachment = nullptr;
		}

		Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();
		m_ColourAttachments.resize(m_ColourAttachmentSpecs.size());

		for (uint32_t i = 0; i < m_ColourAttachments.size(); i++)
		{
			VkFormat format = Utils::NebulaFBFormattoVulkan(m_ColourAttachmentSpecs[i].TextureFormat);

			if (m_Specifications.SwapChainTarget && format == *(VkFormat*)context->GetImageFormat())
			{
				m_ColourAttachments[i] = VulkanImage::CreateImageArray(context->m_Images, context->m_ImageViews);
				continue;
			}
			
			m_ColourAttachments[i] = VulkanImage::CreateImageArray(format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
				m_Specifications.samples, m_Specifications.Width, m_Specifications.Height);
		}

		if (m_DepthAttachmentSpec.TextureFormat != AttachmentTextureFormat::None)
		{
			VkFormat format = Utils::FindDepthFormat();

			m_DepthAttachment = CreateRef<VulkanImage>(format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
				Utils::GetDepthAspectFlags(format), m_Specifications.samples, m_Specifications.Width, m_Specifications.Height);
		}

		m_Framebuffer.resize(context->GetImageCount());
		for (uint32_t imageIndex = 0; imageIndex < m_Framebuffer.size(); imageIndex++)
		{
			std::vector<VkImageView> attachments(m_ColourAttachments.size());
			for (uint32_t i = 0; i < attachments.size(); i++)
				attachments[i] = m_ColourAttachments[i][imageIndex]->GetImageView();

			if (m_DepthAttachmentSpec.TextureFormat != AttachmentTextureFormat::None)
				attachments.push_back(m_DepthAttachment->GetImageView());

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
		}

		if (!m_Specifications.SwapChainTarget)
		{
			m_ImGuiDescriptors.resize(context->GetImageCount());
			for (uint32_t i = 0; i < context->GetImageCount(); i++)
				m_ImGuiDescriptors[i] = ImGui_ImplVulkan_AddTexture(m_ImGuiSampler, m_ColourAttachments[0][i]->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}

		m_StagingBuffer = CreateScope<VulkanBuffer>(m_Specifications.Width * m_Specifications.Height, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}

	void Vulkan_FrameBuffer::Bind() 
	{
		s_BindedInstance = this;

		Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();
		VkCommandBuffer commandBuffer = VulkanAPI::BeginSingleUseCommand();

		for (auto& attachment : m_ColourAttachments)
		{
			Ref<VulkanImage>& image = attachment[context->m_ImageIndex];
			
			if (m_Specifications.SwapChainTarget && image->GetFormat() == VK_FORMAT_UNDEFINED) // Assume it's from swapchain
			{
				// Image doesn't know it's in VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				image->GetLayout() = image->GetLayout() != VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : image->GetLayout();
			}
			
			VulkanAPI::TransitionImageLayout(image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, commandBuffer);
		}

		VulkanAPI::EndSingleUseCommand(commandBuffer);
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

		Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();
		Ref<VulkanImage> image = m_ColourAttachments[attachmentIndex][context->m_ImageIndex];
		
		VkCommandBuffer commandBuffer = VulkanAPI::BeginSingleUseCommand();
		VulkanAPI::TransitionImageLayout(image->GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, commandBuffer);
		Utils::CopyImageToBuffer(commandBuffer, m_StagingBuffer->GetBuffer(), image->GetImage(), m_Specifications.Width, m_Specifications.Height);
		VulkanAPI::TransitionImageLayout(image->GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, commandBuffer);
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
		auto& image = m_DepthAttachment->GetImage();
		VkImageAspectFlags aspectFlags = m_DepthAttachment->GetAspectFlags();

		VkCommandBuffer commandBuffer = VulkanAPI::BeginSingleUseCommand();
		VkClearDepthStencilValue clearValue = { 1.0f, (uint32_t)value };

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

	uint64_t Vulkan_FrameBuffer::GetColourAttachmentRendererID(uint32_t index) const
	{
		Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();
		VulkanAPI::TransitionImageLayout(m_ColourAttachments[index][context->m_ImageIndex], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		return (uint64_t)m_ImGuiDescriptors[context->m_ImageIndex];
	}
}