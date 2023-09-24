#pragma once

#include "Nebula/Core/API.h"
#include "Nebula/Utils/Arrays.h"

#include "RenderPass.h"
#include "Image.h"

namespace Nebula {
	struct FrameBufferSpecification 
	{
		uint32_t Width, Height;
		uint32_t Samples = 1;

		std::vector<AttachmentTextureSpecification> Attachments;
		glm::vec4 ClearColour;
		float DepthClearValue;

		bool SwapChainTarget = false;
		Ref<RenderPass> RenderPass = nullptr;
	};

	class FrameBuffer {
	public:
		virtual ~FrameBuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

		// Optimisation to group clear commands into a single (vulkan) command buffer
		virtual void BeginClear() {};
		virtual void EndClear() {};

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;
		virtual void ClearAttachment(uint32_t attachmentIndex, const glm::vec4& value) = 0;
		virtual void ClearDepthAttachment(int value) {}

		virtual Ref<Image2D> GetColourAttachmentImage(uint32_t index = 0) const = 0;
		virtual uint64_t GetColourAttachmentRendererID(uint32_t index = 0) const = 0;
		
		virtual FrameBufferSpecification& GetFrameBufferSpecifications() = 0;
		virtual const FrameBufferSpecification& GetFrameBufferSpecifications() const = 0;

		static Ref<FrameBuffer> Create(const FrameBufferSpecification& specifications);
	};
}
