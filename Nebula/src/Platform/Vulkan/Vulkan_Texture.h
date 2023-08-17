#pragma once

#include "Nebula/renderer/Texture.h"

#include "VulkanAPI.h"

namespace Nebula {
	class Vulkan_Texture2D : public Texture2D {
	public:
		Vulkan_Texture2D(const TextureSpecification& specification, Buffer data = Buffer());
		~Vulkan_Texture2D();

		const TextureSpecification& GetSpecification() const override { return m_Specification; }

		void SetData(Buffer data) override;
		void SetFilterNearest(bool nearest) override;

		uint32_t GetWidth() const override { return m_Width; }
		uint32_t GetHeight() const override { return m_Height; }
		uint64_t GetImage() const override { return (uint64_t)m_Image->GetImage(); }
		uint64_t GetRendererID() const override { NB_ASSERT(m_Specification.ImGuiUsable); return (uint64_t)m_ImguiDescriptor; }

		void Bind(uint32_t slot) const;
		void Unbind() const;

		bool IsLoaded() const override { return m_IsLoaded; }

		bool operator==(const Texture& other) const override {
			return GetImage() == other.GetImage();
		}
	private:
		TextureSpecification m_Specification;

		bool m_IsLoaded = false;
		uint32_t m_Width, m_Height;
		VkFormat m_Format;

		Ref<VulkanImage> m_Image;
		VkSampler m_Sampler;

		VkDescriptorSet m_ImguiDescriptor = VK_NULL_HANDLE;

		friend class Vulkan_Shader;
	};
}
