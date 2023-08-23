#pragma once

#include "Nebula/Renderer/FrameBuffer.h"

namespace Nebula {
	class OpenGL_FrameBuffer : public FrameBuffer {
	public:
		OpenGL_FrameBuffer(const FrameBufferSpecification& specifications);
		~OpenGL_FrameBuffer(); 

		void Resize(uint32_t width, uint32_t height) override;
		int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		void ClearAttachment(uint32_t attachmentIndex, int value) override;
		void ClearAttachment(uint32_t attachmentIndex, const glm::vec4& value) override;
		
		void Invalidate();

		void Bind() override;
		void Unbind() override;

		Ref<Image2D> GetColourAttachmentImage(uint32_t index) const override { NB_ASSERT(index < m_ColourAttachments.size(), "Index is greater than Array Size"); return nullptr; }
		uint64_t GetColourAttachmentRendererID(uint32_t index) const override { NB_ASSERT(index < m_ColourAttachments.size(), "Index is greater than Array Size"); return m_ColourAttachments[index]; }

		FrameBufferSpecification& GetFrameBufferSpecifications() override { return m_Specifications; }
		const FrameBufferSpecification& GetFrameBufferSpecifications() const override { return m_Specifications; }
	private:
		uint32_t m_RendererID = 0;
		FrameBufferSpecification m_Specifications;

		Array<AttachmentTextureSpecification> m_ColourAttachmentSpecs;
		AttachmentTextureSpecification m_DepthAttachmentSpec = ImageFormat::None;

		std::vector<uint32_t> m_ColourAttachments;
		uint32_t m_DepthAttachment = 0;
	};
}