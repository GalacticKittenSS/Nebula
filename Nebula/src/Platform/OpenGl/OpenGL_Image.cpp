#include "nbpch.h"
#include "OpenGL_Image.h"

namespace Nebula
{
	OpenGL_Image::OpenGL_Image(uint32_t rendererID)
		: m_RendererID(rendererID)
	{
	}

	OpenGL_Image::OpenGL_Image(const ImageSpecification& specification)
		: m_Specification(specification)
	{
	}

	OpenGL_Image::~OpenGL_Image()
	{
	}
}