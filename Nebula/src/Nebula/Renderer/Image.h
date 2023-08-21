#pragma once

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

	struct ImageSpecification
	{
		ImageFormat Format;
		uint32_t Usage;
		
		bool ShaderUsage = true;
		bool ImGuiUsage = true;

		int Samples = 1;
		uint32_t Width;
		uint32_t Height;
	};

	class Image2D
	{
	public:
		~Image2D() = default;

		virtual uint64_t GetImage() const = 0;
		virtual uint64_t GetImageView() const = 0;
		virtual uint64_t GetDescriptorSet() const = 0;

		virtual bool operator==(const Image2D& other) const = 0;

		static Ref<Image2D> Create(const ImageSpecification& specifications);
	};
}