#pragma once

#include "Nebula/Core/Buffer.h"

namespace Nebula
{
	enum class ImageFormat
	{
		None = 0,
		R8,
		RGB8,
		RGBA8,
		RGBA32F,
		BGR8,
		BGRA8,

		//Depth /Stencil
		DEPTH24STENCIL8,

		//Defaults
		Depth = DEPTH24STENCIL8,
		RED_INT = R8
	};

	enum ImageUsage
	{
		TransferSrc = 0x00000001,
		TransferDst = 0x00000002,
		Sampled = 0x00000004,
		Storage = 0x00000008,
		ColourAttachment = 0x00000010,
		DepthStencilAttachment = 0x00000020,
	};

	enum class ImageLayout
	{
		Undefined = 0,
		General = 1,
		ColourAttachment= 2,
		DepthStencilAttachment = 3,
		DepthStencilReadOnly = 4,
		ShaderReadOnly= 5,
		TransferSrc = 6,
		TransferDst = 7,
		PreInitalized = 8,
		DepthReadOnlyStencilAttachment = 1000117000,
		DepthAttachmentStencilReadOnly = 1000117001,
		DepthAttachment = 1000241000,
		DepthReadOnly = 1000241001,
		StencilAttachment = 1000241002,
		StencilReadOnly = 1000241003,
		ReadOnly = 1000314000,
		Attachment = 1000314001,
		PresentSrcKHR = 1000001002,
	};
	
	struct ImageSpecification
	{
		ImageFormat Format;
		uint32_t Usage;
		
		bool ShaderUsage = true;
		bool ImGuiUsage = true;

		uint32_t Samples = 1;
		uint32_t Width;
		uint32_t Height;

		//Debug
		std::string DebugName;
	};

	class Image2D
	{
	public:
		~Image2D() = default;

		virtual uint64_t GetImage() const = 0;
		virtual uint64_t GetImageView() const = 0;
		virtual uint64_t GetDescriptorSet() const = 0;

		virtual Buffer ReadToBuffer() = 0;

		virtual void TransitionImageLayout(ImageLayout oldLayout, ImageLayout newlayout) = 0;
		
		virtual bool operator==(const Image2D& other) const = 0;
		virtual const ImageSpecification& GetSpecification() const = 0;

		static Ref<Image2D> Create(const ImageSpecification& specifications);
	};
}