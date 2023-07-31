#include "nbpch.h"
#include "Texture.h"

#include "Renderer.h"
#include "Platform/OpenGl/OpenGL_Texture.h"
#include "Platform/Vulkan/Vulkan_Texture.h"

namespace Nebula {
	Ref<Texture2D> Texture2D::Create(const TextureSpecification& specification, Buffer data) {
		switch (RendererAPI::GetAPI()) {
			case RendererAPI::API::None:	NB_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:	return CreateRef<OpenGL_Texture2D>(specification, data);
			case RendererAPI::API::Vulkan:	return CreateRef<Vulkan_Texture2D>(specification, data);
		}

		NB_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}
	
	//-----------------------------------------------------//
	/////////////////////////////////////////////////////////
	///////////////////// Sub Texture ///////////////////////
	/////////////////////////////////////////////////////////
	//-----------------------------------------------------//

	Ref<SubTexture2D> SubTexture2D::CreateFromCoords(const Ref<Texture2D>& texture, const glm::vec2& coords, const glm::vec2& cellSize, const glm::vec2& spriteSize) {
		glm::vec2 min = {
			((coords.x + 0) * cellSize.x) / texture->GetWidth(),
			((coords.y + 0) * cellSize.y) / texture->GetHeight()
		};

		glm::vec2 max = {
			((coords.x + spriteSize.x) * cellSize.x) / texture->GetWidth(),
			((coords.y + spriteSize.y) * cellSize.y) / texture->GetHeight()
		};
		
		return CreateRef<SubTexture2D>(texture, min, max);
	}

	SubTexture2D::SubTexture2D(const Ref<Texture2D>& texture, glm::vec2& min, glm::vec2& max): m_Texture(texture) {
		m_TexCoords = new glm::vec2[4];
		m_TexCoords[0] = { min.x, min.y };
		m_TexCoords[1] = { max.x, min.y };
		m_TexCoords[2] = { max.x, max.y };
		m_TexCoords[3] = { min.x, max.y };
	}

	SubTexture2D::~SubTexture2D() {
		delete[] m_TexCoords;
	}
}
