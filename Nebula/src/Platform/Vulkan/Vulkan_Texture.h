#pragma once

#include "Nebula/renderer/Texture.h"

#include "VulkanAPI.h"
#include "Vulkan_Image.h"

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
		Ref<Image2D> GetImage() const override { return m_Image; }
		uint64_t GetRendererID() const override { return m_Image->GetDescriptorSet(); }

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
		
		Ref<Vulkan_Image> m_Image;
		Scope<VulkanBuffer> m_StagingBuffer = nullptr;

		friend class Vulkan_Shader;
	};
}
