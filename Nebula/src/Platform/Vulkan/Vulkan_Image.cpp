#include "nbpch.h"
#include "Vulkan_Image.h"

#include "vk_mem_alloc.h"

#include <backends/imgui_impl_vulkan.h>

#define RETURN_FORMAT_SUPPORTED(format) \
if (FormatSupported(format, VK_IMAGE_TILING_LINEAR, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))\
	return format;\
break\

namespace Nebula
{
	namespace Utils
	{
		VkSampleCountFlagBits GetSampleFlags(int samples)
		{
			switch (samples)
			{
			case 1: return VK_SAMPLE_COUNT_1_BIT;
			case 2: return VK_SAMPLE_COUNT_2_BIT;
			case 4: return VK_SAMPLE_COUNT_4_BIT;
			case 8: return VK_SAMPLE_COUNT_8_BIT;
			case 16: return VK_SAMPLE_COUNT_16_BIT;
			case 32: return VK_SAMPLE_COUNT_32_BIT;
			case 64: return VK_SAMPLE_COUNT_64_BIT;
			}

			NB_ASSERT(false);
			return VK_SAMPLE_COUNT_1_BIT;
		}

		static std::string VKFormatToString(VkFormat format)
		{
			switch (format)
			{
			case VK_FORMAT_R8G8B8_UNORM:	return "VK_FORMAT_R8G8B8_UNORM";
			case VK_FORMAT_R8G8B8A8_UNORM:	return "VK_FORMAT_R8G8B8A8_UNORM";
			}

			return "Unknown";
		}

		uint32_t VulkantoBPP(VkFormat format) {
			switch (format)
			{
			case VK_FORMAT_R8_SINT: return 1;
			case VK_FORMAT_R8G8B8_UNORM: return 3;
			case VK_FORMAT_B8G8R8_UNORM: return 3;
			case VK_FORMAT_R8G8B8A8_UNORM: return 4;
			case VK_FORMAT_B8G8R8A8_UNORM: return 4;
			}

			NB_ASSERT(false, "Unknown Vulkan Format");
			return 0;
		}

		static bool FormatSupported(VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags features)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(VulkanAPI::GetPhysicalDevice(), format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
				return true;

			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
				return true;

			NB_TRACE("Format {} not supported", VKFormatToString(format));
			return false;
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

		VkFormat NebulaToVKImageFormat(ImageFormat format)
		{

			switch (format)
			{
			case ImageFormat::R8:				RETURN_FORMAT_SUPPORTED(VK_FORMAT_R8_SINT);
			case ImageFormat::RGB8:				RETURN_FORMAT_SUPPORTED(VK_FORMAT_R8G8B8_UNORM);
			case ImageFormat::RGBA8:			RETURN_FORMAT_SUPPORTED(VK_FORMAT_R8G8B8A8_UNORM);
			case ImageFormat::BGR8:				RETURN_FORMAT_SUPPORTED(VK_FORMAT_B8G8R8_UNORM);
			case ImageFormat::BGRA8:			RETURN_FORMAT_SUPPORTED(VK_FORMAT_B8G8R8A8_UNORM);
			case ImageFormat::DEPTH24STENCIL8:	return FindDepthFormat();
			}

			NB_ASSERT(FormatSupported(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT), "Format is unknown or not supported!");
			return VK_FORMAT_R8G8B8A8_UNORM;
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

	Vulkan_Image::Vulkan_Image()
		: m_ImageFormat(VK_FORMAT_UNDEFINED), m_AspectFlags(VK_IMAGE_ASPECT_NONE), m_Allocation(nullptr)
	{
	}

	Vulkan_Image::Vulkan_Image(const ImageSpecification& specification)
		: m_Specification(specification)
	{
		bool isDepthFormat = m_Specification.Format == ImageFormat::DEPTH24STENCIL8;

		m_ImageFormat = Utils::NebulaToVKImageFormat(m_Specification.Format);
		m_AspectFlags = isDepthFormat ? Utils::GetDepthAspectFlags(m_ImageFormat) : VK_IMAGE_ASPECT_COLOR_BIT;
		CreateTextureImage(specification.Samples, m_ImageFormat, m_Specification.Usage, m_AspectFlags);

		if (isDepthFormat || !m_Specification.ShaderUsage)
			return;

		CreateSampler();

		m_ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_ImageInfo.imageView = m_ImageView;
		m_ImageInfo.sampler = m_Sampler;

		uint32_t size = m_Specification.Width * m_Specification.Height * Utils::VulkantoBPP(m_ImageFormat);
		m_StagingBuffer = CreateScope<VulkanBuffer>(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	}

	Vulkan_Image::~Vulkan_Image()
	{
		if (!m_Allocation)
			return;

		VulkanAPI::SubmitResource([memory = m_Allocation, image = m_Image, view = m_ImageView, sampler = m_Sampler, descriptor = m_ImGuiDescriptor]()
		{
			vmaDestroyImage(VulkanAPI::s_Allocator, image, memory);
			vkDestroyImageView(VulkanAPI::GetDevice(), view, nullptr);
			vkDestroySampler(VulkanAPI::GetDevice(), sampler, nullptr);
			
			if (descriptor)
				vkFreeDescriptorSets(VulkanAPI::GetDevice(), VulkanAPI::s_DescriptorPool, 1, &descriptor);
		});

		m_Allocation = nullptr;
		m_Image = nullptr;
		m_ImageView = nullptr;
		m_Sampler = nullptr;
		m_ImGuiDescriptor = nullptr;
	}

	void Vulkan_Image::CreateTextureImage(int samples, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect)
	{
		// Image
		{
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = m_Specification.Width;
			imageInfo.extent.height = m_Specification.Height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = format;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = usage;
			imageInfo.samples = Utils::GetSampleFlags(samples);
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

			VkResult result = vmaCreateImage(VulkanAPI::s_Allocator, &imageInfo, &allocInfo, &m_Image, &m_Allocation, nullptr);
			NB_ASSERT(result == VK_SUCCESS, "Failed to create image!");

			VulkanAPI::AttachDebugNameToObject(VK_OBJECT_TYPE_IMAGE, (uint64_t)m_Image, m_Specification.DebugName);
		}
		
		CreateImageView(m_ImageView, format, aspect);
		VulkanAPI::AttachDebugNameToObject(VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)m_ImageView, m_Specification.DebugName);
	}

	void Vulkan_Image::CreateImageView(VkImageView& imageView, VkFormat format, VkImageAspectFlags aspect)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_Image;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format;
		createInfo.subresourceRange.aspectMask = aspect;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView(VulkanAPI::GetDevice(), &createInfo, nullptr, &imageView);
		NB_ASSERT(result == VK_SUCCESS, "Failed to create image view!");
	}

	void Vulkan_Image::CreateSampler()
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

		VkResult result = vkCreateSampler(VulkanAPI::GetDevice(), &samplerInfo, nullptr, &m_Sampler);
		NB_ASSERT(result == VK_SUCCESS, "Failed to create texture sampler!");
	}

	Vulkan_Image::VulkanImageArray Vulkan_Image::CreateImageArray(const ImageSpecification& specification, uint32_t size)
	{
		VulkanImageArray imageArray(size);
		for (uint32_t i = 0; i < size; i++)
			imageArray[i] = CreateRef<Vulkan_Image>(specification);

		return imageArray;
	}

	Vulkan_Image::VulkanImageArray Vulkan_Image::CreateImageArray(const std::vector<VkImage>& images, const std::vector<VkImageView>& imageViews,
		uint32_t width, uint32_t height, VkFormat format)
	{
		size_t arraySize = glm::max(images.size(), imageViews.size());
		VulkanImageArray imageArray(arraySize);

		for (uint32_t i = 0; i < arraySize; i++)
		{
			imageArray[i] = CreateRef<Vulkan_Image>();
			imageArray[i]->m_Image = images[i];
			imageArray[i]->m_ImageView = imageViews[i];
			imageArray[i]->m_ImageFormat = format;
			imageArray[i]->m_Specification.Width = width;
			imageArray[i]->m_Specification.Height = height;
			imageArray[i]->m_AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT; // Assume these images are coming from swapchain
		}

		return imageArray;
	}

	Buffer Vulkan_Image::ReadToBuffer()
	{
		VkCommandBuffer commandBuffer = VulkanAPI::BeginSingleUseCommand();
		VulkanAPI::TransitionImageLayout(m_Image, VK_IMAGE_ASPECT_COLOR_BIT, ImageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, commandBuffer);
		Utils::CopyImageToBuffer(commandBuffer, m_StagingBuffer->GetBuffer(), m_Image, m_Specification.Width, m_Specification.Height);

		ImageLayout = ImageLayout != VK_IMAGE_LAYOUT_UNDEFINED ? ImageLayout : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VulkanAPI::TransitionImageLayout(m_Image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, ImageLayout, commandBuffer);
		VulkanAPI::EndSingleUseCommand(commandBuffer);

		Buffer buffer;
		buffer.Size = m_Specification.Width * m_Specification.Height * Utils::VulkantoBPP(m_ImageFormat);
		buffer.Data = (uint8_t*)m_StagingBuffer->GetMemory();
		return buffer;
	}
	
	typedef ImageLayout ImageLayout_t;
	void Vulkan_Image::TransitionImageLayout(ImageLayout_t oldLayout, ImageLayout_t newlayout)
	{
		VkCommandBuffer commandBuffer = VulkanAPI::IsRecording() ? VulkanAPI::GetCommandBuffer() : nullptr;
		VulkanAPI::TransitionImageLayout(m_Image, m_AspectFlags, (VkImageLayout)oldLayout, (VkImageLayout)newlayout, commandBuffer);
		ImageLayout = (VkImageLayout)newlayout;
	}

	uint64_t Vulkan_Image::GetDescriptorSet()
	{
		if (!m_ImGuiDescriptor)
			m_ImGuiDescriptor = ImGui_ImplVulkan_AddTexture(m_Sampler, m_ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		return (uint64_t)m_ImGuiDescriptor;
	}
}