#include "nbpch.h"
#include "Vulkan_Texture.h"

#include "VulkanAPI.h"
#include "Vulkan_Shader.h"
#include "Vulkan_Image.h"

#include "Nebula/Scene/SceneRenderer.h"

namespace Nebula {
	namespace Utils 
	{
		uint32_t VulkantoBPP(VkFormat format);

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

		ImageSpecification imageSpec;
		imageSpec.Format = m_Specification.Format;
		imageSpec.Usage = ImageUsage::TransferSrc | ImageUsage::TransferDst | ImageUsage::Sampled;
		imageSpec.Samples = 1;
		imageSpec.Width = m_Specification.Width;
		imageSpec.Height = m_Specification.Height;
		
		if (!m_Specification.DebugName.empty())
			imageSpec.DebugName = m_Specification.DebugName + "-Image";

		m_Image = CreateRef<Vulkan_Image>(imageSpec);

		uint32_t bpp = Utils::VulkantoBPP(m_Image->GetFormat());
		uint32_t bufferSize = m_Width * m_Height * bpp;
		m_StagingBuffer = CreateScope<VulkanBuffer>(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		
		if (data)
		{
			SetData(data);
			m_IsLoaded = true;
		}
	}

	Vulkan_Texture2D::~Vulkan_Texture2D() 
	{
		NB_PROFILE_FUNCTION();
	}

	void Vulkan_Texture2D::SetData(Buffer data) 
	{
		NB_PROFILE_FUNCTION();

		if (data.Size != m_StagingBuffer->GetSize())
		{
			// Add Alpha Padding
			uint32_t offset = 0;
			uint32_t channels = (uint32_t)(data.Size / (m_Width * m_Height));
			NB_ASSERT(channels >= 3, "Texture2D::SetData() => Expected 3 or more Channels");

			for (uint32_t i = 0; i < data.Size; i += channels)
			{
				unsigned char padded[] = { data.Data[i] , data.Data[i + 1], data.Data[i + 2], 255 };
				m_StagingBuffer->SetData(padded, sizeof(padded), offset);
				offset += sizeof(padded);
			}
		}
		else
		{
			m_StagingBuffer->SetData(data.Data, (uint32_t)data.Size);
		}

		VulkanAPI::TransitionImageLayout(m_Image->GetVulkanImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		Utils::CopyBufferToImage(m_StagingBuffer->GetBuffer(), m_Image->GetVulkanImage(), m_Width, m_Height);
		VulkanAPI::TransitionImageLayout(m_Image->GetVulkanImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void Vulkan_Texture2D::SetFilterNearest(bool nearest) 
	{
		// Recreate Sampler
	}

	void Vulkan_Texture2D::Bind(uint32_t slot) const 
	{
		NB_PROFILE_FUNCTION();

		// Update Descriptor Set
		Vulkan_Shader::SetTexture(slot, m_Image->GetVulkanImageInfo());
	}

	void Vulkan_Texture2D::Unbind() const 
	{
		NB_PROFILE_FUNCTION();
	}
}