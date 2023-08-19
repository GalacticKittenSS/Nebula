#pragma once

namespace Nebula
{
	enum class AttachmentTextureFormat 
	{
		None = 0,

		//Color
		RGBA8,
		RED_INT,

		//Depth /Stencil
		DEPTH24STENCIL8,

		//Defaults
		Depth = DEPTH24STENCIL8
	};

	struct AttachmentTextureSpecification 
	{
		AttachmentTextureSpecification() = default;
		AttachmentTextureSpecification(AttachmentTextureFormat format) :
			TextureFormat(format) { }

		AttachmentTextureFormat TextureFormat = AttachmentTextureFormat::None;
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