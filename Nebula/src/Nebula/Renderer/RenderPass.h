#pragma once

#include "Image.h"

namespace Nebula
{
	struct AttachmentTextureSpecification 
	{
		AttachmentTextureSpecification() = default;
		AttachmentTextureSpecification(ImageFormat format) :
			TextureFormat(format) { }
		AttachmentTextureSpecification(ImageFormat format, ImageLayout original, ImageLayout _final) :
			TextureFormat(format), OriginalLayout(original), FinalLayout(_final) { }

		ImageFormat TextureFormat = ImageFormat::None;
		ImageLayout OriginalLayout = ImageLayout::Undefined;
		ImageLayout FinalLayout = ImageLayout::Undefined;
	};

	struct RenderPassSpecification
	{
		std::vector<AttachmentTextureSpecification> Attachments;
		bool SingleWrite = false;
		bool ClearOnLoad = false;
	};

	class RenderPass
	{
	public:
		virtual ~RenderPass() = default;

		virtual void Bind() {};
		virtual void Unbind() {};
		
		virtual uint64_t GetRenderPass() const { return -1; };

		virtual RenderPassSpecification& GetRenderPassSpecifications() { return s_DefaultSpecification; }
		virtual const RenderPassSpecification& GetRenderPassSpecifications() const { return s_DefaultSpecification; }

		static Ref<RenderPass> Create(const RenderPassSpecification& attachments);
	private:
		static RenderPassSpecification s_DefaultSpecification;
	};
}