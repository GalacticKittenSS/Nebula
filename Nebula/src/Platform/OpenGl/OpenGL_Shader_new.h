#pragma once

#include "Nebula/renderer/Shader.h"
#include "Nebula/Maths/Maths.h"

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
		void SetIntArray(const std::string& name, int* values, uint32_t count) override;
		void SetFloat(const std::string& name, const float value) override;
		void SetMat4(const std::string& name, const mat4& value) override;
		void SetFloat3(const std::string& name, const vec3& values) override;
		void SetFloat4(const std::string& name, const vec4& values) override;

		void UploadUniformInt(const std::string& name, const int value);
		void UploadUniformIntArray(const std::string& name, int* values, uint32_t count);
		
		//void UploadUniformMat3(const std::string& name, const mat3& value);
		void UploadUniformMat4(const std::string& name, const mat4& value);

		void UploadUniformFloat(const std::string& name, const float values);
		void UploadUniformFloat2(const std::string& name, const vec2& values);
		void UploadUniformFloat3(const std::string& name, const vec3& values);
		void UploadUniformFloat4(const std::string& name, const vec4& values);
	private:
		std::string ReadFile(const std::string& path);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& shaderSrc);
		void CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources);
		void CompileOrGetOpenGLBinaries();
		void CreateProgram();
		void Reflect(GLenum stage, const std::vector<uint32_t>& shaderData);
	private:
		uint32_t m_RendererID;
		std::string m_FilePath;
		std::string m_Name;

		std::unordered_map<GLenum, std::vector<uint32_t>> m_VulkanSPIRV;
		std::unordered_map<GLenum, std::vector<uint32_t>> m_OpenGLSPIRV;

		std::unordered_map<GLenum, std::string> m_OpenGLSourceCode;
	};
}