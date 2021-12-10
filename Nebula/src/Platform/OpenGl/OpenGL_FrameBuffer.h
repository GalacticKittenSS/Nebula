#pragma once

#include "Nebula/Renderer/FrameBuffer.h"

namespace Nebula {
	class OpenGL_FrameBuffer : public FrameBuffer {
	public:
		OpenGL_FrameBuffer(const FrameBufferSpecification& specifications);
		~OpenGL_FrameBuffer(); 

		void Resize(uint32_t width, uint32_t height) override;
		void Invalidate();

		void Bind() override;
		void Unbind() override;

		uint32_t GetColourAttachmentRendererID() const override { return m_colourAttachment; }

		FrameBufferSpecification& GetFrameBufferSpecifications() override { return m_Specifications; }
		const FrameBufferSpecification& GetFrameBufferSpecifications() const override { return m_Specifications; }
	private:
		uint32_t m_RendererID = 0;
		uint32_t m_colourAttachment = 0, m_DepthAttachment = 0;
		FrameBufferSpecification m_Specifications;
	};
}