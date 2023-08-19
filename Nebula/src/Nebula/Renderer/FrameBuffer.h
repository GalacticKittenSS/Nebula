#pragma once

#include "Nebula/Core/API.h"
#include "Nebula/Utils/Arrays.h"

#include "RenderPass.h"

namespace Nebula {
	struct FrameBufferAttachmentSpecification {
		FrameBufferAttachmentSpecification() = default;
		FrameBufferAttachmentSpecification(std::initializer_list<AttachmentTextureSpecification> attachments) :
			Attachments{ attachments } { }

		Array<AttachmentTextureSpecification> Attachments;
	};

	struct FrameBufferSpecification {
		uint32_t Width, Height;
		FrameBufferAttachmentSpecification Attachments;
		Ref<RenderPass> RenderPass = nullptr;
		uint32_t samples = 1;

		bool SwapChainTarget = false;
	};

	class FrameBuffer {
	public:
		virtual ~FrameBuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;
		virtual void ClearAttachment(uint32_t attachmentIndex, const glm::vec4& value) = 0;
		virtual void ClearDepthAttachment(int value) {}

		virtual uint64_t GetColourAttachmentRendererID(uint32_t index = 0) const = 0;

		virtual FrameBufferSpecification& GetFrameBufferSpecifications() = 0;
		virtual const FrameBufferSpecification& GetFrameBufferSpecifications() const = 0;

		static Ref<FrameBuffer> Create(const FrameBufferSpecification& specifications);
	};
}
