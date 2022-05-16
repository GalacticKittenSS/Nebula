#pragma once

#include "Nebula/Core/API.h"
#include "Nebula/Utils/Arrays.h"

namespace Nebula {
	enum class FramebufferTextureFormat {
		None = 0,

		//Color
		RGBA8, 
		RED_INT,

		//Depth /Stencil
		DEPTH24STENCIL8,

		//Defaults
		Depth = DEPTH24STENCIL8
	};

	struct FramebufferTextureSpecification {
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(FramebufferTextureFormat format):
			TextureFormat(format) { }

		FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
	};

	struct FrameBufferAttachmentSpecification {
		FrameBufferAttachmentSpecification() = default;
		FrameBufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments) :
			Attachments(attachments) { }

		Array<FramebufferTextureSpecification> Attachments;
	};

	struct FrameBufferSpecification {
		uint32_t Width, Height;
		FrameBufferAttachmentSpecification Attachments;
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

		virtual uint32_t GetColourAttachmentRendererID(uint32_t index = 0) const = 0;

		virtual FrameBufferSpecification& GetFrameBufferSpecifications() = 0;
		virtual const FrameBufferSpecification& GetFrameBufferSpecifications() const = 0;

		static Ref<FrameBuffer> Create(const FrameBufferSpecification& specifications);
	};
}