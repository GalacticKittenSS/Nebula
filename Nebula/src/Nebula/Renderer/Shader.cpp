#include "nbpch.h"
#include "Shader.h"

#include "Renderer.h"
#include "Platform/OpenGl/OpenGL_Shader.h"
#include "Platform/Vulkan/Vulkan_Shader.h"

namespace Nebula {
	Ref<Shader> Shader::Create(const std::string& path) {
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:	NB_ASSERT(false, "RendererAPI::None is currently not supported!");
		case RendererAPI::API::OpenGL:	return CreateRef<OpenGL_Shader>(path);
		case RendererAPI::API::Vulkan:	return CreateRef<Vulkan_Shader>(path);
		}

		NB_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<Shader> Shader::Create(const std::string& name, const std::string& vertSrc, const std::string& fragSrc) {
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:	NB_ASSERT(false, "RendererAPI::None is currently not supported!");
			case RendererAPI::API::OpenGL:	return CreateRef<OpenGL_Shader>(name, vertSrc, fragSrc);
			case RendererAPI::API::Vulkan:	return CreateRef<Vulkan_Shader>(name, vertSrc, fragSrc);
		}

		NB_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader) {
		NB_ASSERT(m_Shaders.find(name) == m_Shaders.end(), "Shader already exists!");
		m_Shaders[name] = shader;
	}

	void ShaderLibrary::Add(const Ref<Shader>& shader) {
		auto& name = shader->GetName();
		Add(name, shader);
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& filepath) {
		auto shader = Shader::Create(filepath);
		Add(shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& filepath, const std::string& name) {
		auto shader = Shader::Create(filepath);
		Add(name, shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Get(const std::string& name) {
		NB_ASSERT(m_Shaders.find(name) != m_Shaders.end(), "Shader not found!");
		return m_Shaders[name];
	}

}
