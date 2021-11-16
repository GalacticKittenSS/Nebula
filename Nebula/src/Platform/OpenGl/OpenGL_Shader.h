#pragma once

#include "Nebula/renderer/Shader.h"

#include <glm/glm.hpp>

typedef unsigned int GLenum;

namespace Nebula {
	class OpenGL_Shader: public Shader {
	public:
		OpenGL_Shader(const std::string& path);
		OpenGL_Shader(const std::string& name, const std::string& vertSrc, const std::string& fragSrc);
		~OpenGL_Shader();

		void Bind() const override;
		void Unbind() const override;

		const std::string& GetName() const override { return m_Name; }

		void SetInt(const std::string& name, const int value) override;
		void SetFloat(const std::string& name, const float value) override;
		void SetMat4(const std::string& name, const glm::mat4& value) override;
		void SetFloat3(const std::string& name, const glm::vec3& values) override;
		void SetFloat4(const std::string& name, const glm::vec4& values) override;

		void UploadUniformInt(const std::string& name, const int value);
		
		void UploadUniformMat3(const std::string& name, const glm::mat3& value);
		void UploadUniformMat4(const std::string& name, const glm::mat4& value);

		void UploadUniformFloat(const std::string& name, const float values);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& values);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& values);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& values);
	private:
		std::string ReadFile(const std::string& path);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& shaderSrc);
		bool Compile(std::unordered_map<GLenum, std::string> sources);
	private:
		uint32_t m_RendererID;
		std::string m_Name;
	};
}