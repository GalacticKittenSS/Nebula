#pragma once

#include "Nebula/Renderer/FrameBuffer.h"

namespace Nebula {
	class OpenGL_FrameBuffer : public FrameBuffer {
	public:
		OpenGL_FrameBuffer(const FrameBufferSpecification& specifications);
		~OpenGL_FrameBuffer(); 

		void Invalidate();

		void Bind() override;
		void Unbind() override;

		uint32_t GetColourAttachmentRendererID() const override { return m_colourAttachment; }

		FrameBufferSpecification& GetFrameBufferSpecifications() override { return m_Specifications; }
		const FrameBufferSpecification& GetFrameBufferSpecifications() const override { return m_Specifications; }
	private:
		uint32_t m_RendererID;
		uint32_t m_colourAttachment, m_DepthAttachment;
		FrameBufferSpecification m_Specifications;
	};
}