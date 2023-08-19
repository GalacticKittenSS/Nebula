#pragma once

#include "Nebula/Renderer/FrameBuffer.h"

#include "VulkanAPI.h"

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

		uint64_t GetColourAttachmentRendererID(uint32_t index) const override;

		FrameBufferSpecification& GetFrameBufferSpecifications() override { return m_Specifications; }
		const FrameBufferSpecification& GetFrameBufferSpecifications() const override { return m_Specifications; }

		static Vulkan_FrameBuffer* GetActiveInstance() { return s_BindedInstance; }
	private:
		void CreateRenderPass();
		void ClearAttachment(uint32_t attachmentIndex, VkClearColorValue clearValue);
	private:
		FrameBufferSpecification m_Specifications;
		Array<AttachmentTextureSpecification> m_ColourAttachmentSpecs;
		AttachmentTextureSpecification m_DepthAttachmentSpec = AttachmentTextureFormat::None;

		std::vector<VkFramebuffer> m_Framebuffer;
		
		std::vector<VulkanImageArray> m_ColourAttachments;
		Ref<VulkanImage> m_DepthAttachment;

		VkSampler m_ImGuiSampler = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> m_ImGuiDescriptors;

		static Vulkan_FrameBuffer* s_BindedInstance;
		friend class Vulkan_RenderPass;
		friend class ImGuiLayer;
	};
}