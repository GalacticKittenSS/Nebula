#include "nbpch.h"
#include "Vulkan_Texture.h"

#include "VulkanAPI.h"
#include "Vulkan_Shader.h"

#include "Nebula/Scene/SceneRenderer.h"

#define RETURN_FORMAT_SUPPORTED(format) \
if (FormatSupported(format, VK_IMAGE_TILING_LINEAR, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))\
	return format;\
break\

namespace Nebula {
	namespace Utils 
	{
		static uint32_t VulkantoBPP(VkFormat format) {
			switch (format)
			{
			case VK_FORMAT_R8_SRGB: return 1;
			case VK_FORMAT_R8G8B8_SRGB: return 3;
			case VK_FORMAT_R8G8B8A8_SRGB: return 4;
			}

			NB_ASSERT(false, "Unknown OpenGL Format");
			return 0;
		}

		static std::string VKFormatToString(VkFormat format)
		{
			switch (format)
			{
			case VK_FORMAT_R8G8B8_SRGB:		return "VK_FORMAT_R8G8B8_SRGB";
			case VK_FORMAT_R8G8B8A8_SRGB:	return "VK_FORMAT_R8G8B8A8_SRGB";
			}

			return "Unknown";
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

		static VkFormat NebulaToVKDataFormat(ImageFormat format)
		{
			
			switch (format)
			{
			case ImageFormat::RGB8:		RETURN_FORMAT_SUPPORTED(VK_FORMAT_R8G8B8_SRGB);
			case ImageFormat::RGBA8:	RETURN_FORMAT_SUPPORTED(VK_FORMAT_R8G8B8A8_SRGB);
			}

			NB_ASSERT(FormatSupported(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_LINEAR, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT), "Format is unknown or not supported!");
			return VK_FORMAT_R8G8B8A8_SRGB;
		}

		static void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
		{
			VkCommandBuffer commandBuffer = VulkanAPI::BeginSingleUseCommand();

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

			vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			VulkanAPI::EndSingleUseCommand(commandBuffer);
		}
	}

	Vulkan_Texture2D::Vulkan_Texture2D(const TextureSpecification& specification, Buffer data)
		: m_Specification(specification), m_Width(specification.Width), m_Height(specification.Height)
	{
		NB_PROFILE_FUNCTION();

		m_Format = Utils::NebulaToVKDataFormat(specification.Format);

		m_Image = CreateScope<VulkanImage>(m_Format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
			1, m_Specification.Width, m_Specification.Height);

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

		if (data)
		{
			SetData(data);
			m_IsLoaded = true;
		}

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_Image->GetImageView();
		imageInfo.sampler = m_Sampler;

		Ref<Vulkan_Shader> shader = std::static_pointer_cast<Vulkan_Shader>(SceneRenderer::GetShader());

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = shader->m_DescriptorSet;
		descriptorWrite.dstBinding = 1;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(VulkanAPI::GetDevice(), 1, &descriptorWrite, 0, nullptr);
	}

	Vulkan_Texture2D::~Vulkan_Texture2D() 
	{
		NB_PROFILE_FUNCTION();

		vkDestroySampler(VulkanAPI::GetDevice(), m_Sampler, nullptr);
	}

	void Vulkan_Texture2D::SetData(Buffer data) 
	{
		NB_PROFILE_FUNCTION();

		uint32_t bpp = Utils::VulkantoBPP(m_Format);
		//NB_ASSERT(data.Size == m_Width * m_Height * bpp, "Data must be Entire Texture");

		VulkanBuffer stagingBuffer = VulkanBuffer(m_Width * m_Height * bpp, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		
		if (data.Size != m_Width * m_Height * bpp)
		{
			// Add Alpha Padding
			uint32_t offset = 0;
			for (uint32_t i = 0; i < data.Size; i += data.Size / (m_Width * m_Height))
			{
				unsigned char padded[] = { data.Data[i] , data.Data[i + 1], data.Data[i + 2], 255 };
				stagingBuffer.SetData(padded, sizeof(padded), offset);
				offset += sizeof(padded);
			}
		}
		else
		{
			stagingBuffer.SetData(data.Data, data.Size);
		}

		VulkanAPI::TransitionImageLayout(m_Image->GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		Utils::CopyBufferToImage(stagingBuffer.GetBuffer(), m_Image->GetImage(), m_Width, m_Height);
		VulkanAPI::TransitionImageLayout(m_Image->GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void Vulkan_Texture2D::SetFilterNearest(bool nearest) 
	{
	}

	void Vulkan_Texture2D::Bind(uint32_t slot = 0) const {
		NB_PROFILE_FUNCTION();
	}

	void Vulkan_Texture2D::Unbind() const {
		NB_PROFILE_FUNCTION();

	}
}