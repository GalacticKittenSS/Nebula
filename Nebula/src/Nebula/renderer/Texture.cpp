#include "nbpch.h"
#include "Texture.h"

#include "Renderer.h"
#include "Platform/OpenGl/OpenGL_Texture.h"

namespace Nebula {
	Ref<Texture2D> Texture2D::Create(const std::string& path) {
		switch (RendererAPI::GetAPI()) {
			case RendererAPI::API::None:	NB_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:	return CreateRef<OpenGL_Texture2D>(path);
		}

		NB_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height) {
		switch (RendererAPI::GetAPI()) {
			case RendererAPI::API::None:	NB_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:	return CreateRef<OpenGL_Texture2D>(width, height);
		}

		NB_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}
}