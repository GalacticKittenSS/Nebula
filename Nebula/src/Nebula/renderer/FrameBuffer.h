#pragma once

#include "Nebula/Core/API.h"

namespace Nebula {
	struct FrameBufferSpecification {
		uint32_t Width, Height;
		uint32_t samples = 1;

		bool SwapChainTarget = false;
	};

	class FrameBuffer {
	public:
		virtual ~FrameBuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual uint32_t GetColourAttachmentRendererID() const = 0;

		virtual FrameBufferSpecification& GetFrameBufferSpecifications() = 0;
		virtual const FrameBufferSpecification& GetFrameBufferSpecifications() const = 0;

		static Ref<FrameBuffer> Create(const FrameBufferSpecification& specifications);
	};
}