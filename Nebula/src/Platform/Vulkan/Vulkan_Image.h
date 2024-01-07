#pragma once

#include "Nebula/Renderer/Image.h"

#include "VulkanAPI.h"

namespace Nebula
{
	class Vulkan_Image : public Image2D
	{
	public:
		Vulkan_Image();
		Vulkan_Image(const ImageSpecification& specification);
		~Vulkan_Image();

		using VulkanImageArray = std::vector<Ref<Vulkan_Image>>;
		static VulkanImageArray CreateImageArray(const ImageSpecification& specification, uint32_t size);
		static VulkanImageArray CreateImageArray(const std::vector<VkImage>& images, const std::vector<VkImageView>& imageViews,
			uint32_t width, uint32_t height, VkFormat format);

		uint64_t GetImage() const override { return (uint64_t)m_Image; }
		uint64_t GetImageView() const override { return (uint64_t)m_ImageView; }
		uint64_t GetDescriptorSet() const override { return (uint64_t)m_ImGuiDescriptor; }

		Buffer ReadToBuffer() override;
		void TransitionImageLayout(ImageLayout oldLayout, ImageLayout newlayout) override;
		
		const ImageSpecification& GetSpecification() const override { return m_Specification; }

		inline const VkImage& GetVulkanImage() const { return m_Image; }
		inline const VkImageView& GetVulkanImageView() const { return m_ImageView; }
		
		inline const VkFormat& GetFormat() const { return m_ImageFormat; }
		inline const VkImageAspectFlags& GetAspectFlags() const { return m_AspectFlags; }
		inline const VkDescriptorImageInfo& GetVulkanImageInfo() const { return m_ImageInfo; }

		bool operator==(const Image2D& other) const override {
			return GetImageView() == other.GetImageView();
		}
		
		VkImageLayout ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	private:
		void CreateTextureImage(int samples, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect);
		void CreateSampler();
		void CreateImageView(VkImageView& imageView, VkFormat format, VkImageAspectFlags aspect);
	private:
		ImageSpecification m_Specification;
		
		VkImage m_Image;
		VkImageView m_ImageView;
		VkSampler m_Sampler = VK_NULL_HANDLE;
		
		VmaAllocation m_Allocation;

		VkDescriptorImageInfo m_ImageInfo;
		VkDescriptorSet m_ImGuiDescriptor = VK_NULL_HANDLE;

		VkFormat m_ImageFormat;
		VkImageAspectFlags m_AspectFlags;
	};
}