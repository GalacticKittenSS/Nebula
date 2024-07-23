#pragma once

#include "Nebula/renderer/Shader.h"
#include "Nebula/Maths/Maths.h"

typedef unsigned int GLenum;

namespace Nebula {
	class OpenGL_DescriptorSet : public DescriptorSet {
	public:
		OpenGL_DescriptorSet(uint32_t shaderID);

		void SetResource(const std::string& name, Ref<UniformBuffer> uniformBuffer) override {};
		void SetResource(const std::string& name, Ref<Texture2D> texture, uint32_t slot = 0) override {};
	
		void UploadUniformInt(const std::string& name, const int value) override;
		void UploadUniformIntArray(const std::string& name, int* values, uint32_t count) override;

		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) override;
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) override;

		void UploadUniformFloat(const std::string& name, const float values) override;
		void UploadUniformFloat2(const std::string& name, const glm::vec2& values) override;
		void UploadUniformFloat3(const std::string& name, const glm::vec3& values) override;
		void UploadUniformFloat4(const std::string& name, const glm::vec4& values) override;
	private:
		uint32_t m_ShaderID;
	};

	class OpenGL_Shader: public Shader {
	public:
		OpenGL_Shader(const std::string& path);
		OpenGL_Shader(const std::string& name, const std::string& vertSrc, const std::string& fragSrc);
		~OpenGL_Shader();

		void Bind() const override;
		void Unbind() const override;

		const std::string& GetName() const override { return m_Name; }
		Ref<DescriptorSet> AllocateDescriptorSets() const { return m_DescriptorSet; };

		void SetInt(const std::string& name, int value) override;
		void SetIntArray(const std::string& name, int* values, uint32_t count) override;
		void SetFloat(const std::string& name, float value) override;
		void SetFloat2(const std::string& name, const glm::vec2& value) override;
		void SetFloat3(const std::string& name, const glm::vec3& value) override;
		void SetFloat4(const std::string& name, const glm::vec4& value) override;
		void SetMat4(const std::string& name, const glm::mat4& value) override;
	private:
		std::string ReadFile(const std::string& filepath);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);

		void CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources);
		void CompileOrGetOpenGLBinaries();
		void CreateProgram();

		void Reflect(GLenum stage, const std::vector<uint32_t>& shaderData);
	private:
		uint32_t m_RendererID;
		std::string m_FilePath;
		std::string m_Name;

		Ref<OpenGL_DescriptorSet> m_DescriptorSet;

		std::unordered_map<GLenum, std::vector<uint32_t>> m_VulkanSPIRV;
		std::unordered_map<GLenum, std::vector<uint32_t>> m_OpenGLSPIRV;

		std::unordered_map<GLenum, std::string> m_OpenGLSourceCode;
	};
}