#pragma once

#include "Image.h"

namespace Nebula
{
	struct AttachmentTextureSpecification 
	{
		AttachmentTextureSpecification() = default;
		AttachmentTextureSpecification(ImageFormat format) :
			TextureFormat(format) { }

		ImageFormat TextureFormat = ImageFormat::None;
	};

	struct RenderPassSpecification
	{
		std::vector<AttachmentTextureSpecification> Attachments;
		bool ClearOnLoad = false;
		bool ShaderOnly = true;
	};

	class RenderPass
	{
	public:
		virtual ~RenderPass() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;
		
		virtual uint64_t GetRenderPass() const = 0;

		virtual RenderPassSpecification& GetRenderPassSpecifications() = 0;
		virtual const RenderPassSpecification& GetRenderPassSpecifications() const = 0;

		static Ref<RenderPass> Create(const RenderPassSpecification& attachments);
	};
}