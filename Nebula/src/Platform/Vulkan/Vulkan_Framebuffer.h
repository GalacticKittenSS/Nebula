#pragma once

#include "Nebula/Renderer/FrameBuffer.h"

#include "VulkanAPI.h"
#include "Vulkan_Image.h"

namespace Nebula {
	class Vulkan_FrameBuffer : public FrameBuffer {
	public:
		Vulkan_FrameBuffer(const FrameBufferSpecification& specifications);
		~Vulkan_FrameBuffer();

		void Resize(uint32_t width, uint32_t height) override;
		int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		void ClearAttachment(uint32_t attachmentIndex, int value) override;
		void ClearAttachment(uint32_t attachmentIndex, const glm::vec4& value) override;
		void ClearDepthAttachment(int value);

		void Invalidate();

		void Bind() override;
		void Unbind() override;

		Ref<Image2D> GetColourAttachmentRendererID(uint32_t index) const override;

		FrameBufferSpecification& GetFrameBufferSpecifications() override { return m_Specifications; }
		const FrameBufferSpecification& GetFrameBufferSpecifications() const override { return m_Specifications; }

		static Vulkan_FrameBuffer* GetActiveInstance() { return s_BindedInstance; }
	private:
		void CreateRenderPass();
		void ClearAttachment(uint32_t attachmentIndex, VkClearColorValue clearValue);
	private:
		FrameBufferSpecification m_Specifications;
		Array<AttachmentTextureSpecification> m_ColourAttachmentSpecs;
		AttachmentTextureSpecification m_DepthAttachmentSpec = ImageFormat::None;

		std::vector<VkFramebuffer> m_Framebuffer;
		
		std::vector<Vulkan_Image::VulkanImageArray> m_ColourAttachments;
		Ref<Vulkan_Image> m_DepthAttachment;

		Scope<VulkanBuffer> m_StagingBuffer = nullptr;

		static Vulkan_FrameBuffer* s_BindedInstance;
		friend class Vulkan_RenderPass;
		friend class ImGuiLayer;
	};
}