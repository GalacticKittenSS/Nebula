#pragma once

#include "Nebula/Renderer/Image.h"

namespace Nebula
{
	class OpenGL_Image : public Image2D
	{
	public:
		OpenGL_Image(uint32_t rendererID);
		OpenGL_Image(const ImageSpecification& specification);
		~OpenGL_Image();
		
		uint64_t GetImage() const override { return (uint64_t)m_RendererID; }
		uint64_t GetImageView() const override { return (uint64_t)m_RendererID; }
		uint64_t GetDescriptorSet() override { return (uint64_t)m_RendererID; }

		Buffer ReadToBuffer() override { return {}; }
		void TransitionImageLayout(ImageLayout oldLayout, ImageLayout newlayout) override {};
		
		const ImageSpecification& GetSpecification() const override { return m_Specification; }

		bool operator==(const Image2D& other) const override {
			return GetImageView() == other.GetImageView();
		}
	private:
		ImageSpecification m_Specification;
		uint32_t m_RendererID;
	};
}