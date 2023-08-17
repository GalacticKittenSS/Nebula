#pragma once

#include "Framebuffer.h"

namespace Nebula
{
	struct RenderPassSpecifications
	{
		std::vector<FramebufferTextureSpecification> Attachments;
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

		static Ref<RenderPass> Create(const RenderPassSpecifications& attachments);
	};
}