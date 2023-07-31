#pragma once

#include "Nebula/renderer/Shader.h"
#include "Nebula/Maths/Maths.h"

#include <unordered_map>
#include <vulkan/vulkan.h>

namespace Nebula {
	class Vulkan_Shader : public Shader {
	public:
		Vulkan_Shader(const std::string& path);
		Vulkan_Shader(const std::string& name, const std::string& vertSrc, const std::string& fragSrc);
		~Vulkan_Shader();

		void Bind() const override;
		void Unbind() const override;

		const std::string& GetName() const override { return m_Name; }

		void SetInt(const std::string& name, int value) override;
		void SetIntArray(const std::string& name, int* values, uint32_t count) override;
		void SetFloat(const std::string& name, float value) override;
		void SetFloat2(const std::string& name, const glm::vec2& value) override;
		void SetFloat3(const std::string& name, const glm::vec3& value) override;
		void SetFloat4(const std::string& name, const glm::vec4& value) override;
		void SetMat4(const std::string& name, const glm::mat4& value) override;

		void UploadUniformInt(const std::string& name, const int value);
		void UploadUniformIntArray(const std::string& name, int* values, uint32_t count);

		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

		void UploadUniformFloat(const std::string& name, const float values);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& values);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& values);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& values);
	private:
		std::string ReadFile(const std::string& filepath);
		std::unordered_map<VkShaderStageFlagBits, std::string> PreProcess(const std::string& source);

		void CompileOrGetVulkanBinaries(const std::unordered_map<VkShaderStageFlagBits, std::string>& shaderSources);
		void Reflect(VkShaderStageFlagBits stage, const std::vector<uint32_t>& shaderData);

		VkShaderModule CreateShaderModule(const std::vector<uint32_t>& code);
		void CreatePipeline(VkPipelineShaderStageCreateInfo shaderStages[]);
	private:
		uint32_t m_RendererID;
		std::string m_FilePath;
		std::string m_Name;

		std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>> m_VulkanSPIRV;
		std::unordered_map<VkShaderStageFlagBits, std::vector<char>> m_ShaderCode;

		VkPipeline m_GraphicsPipeline;
		VkPipelineLayout m_PipelineLayout;
		VkDescriptorSetLayout m_DescriptorSetLayout;
		VkDescriptorPool m_DescriptorPool;
		VkDescriptorSet m_DescriptorSet;
		
		friend class Vulkan_RendererAPI;
		friend class Vulkan_UniformBuffer;
		friend class Vulkan_Texture2D;
	};
}