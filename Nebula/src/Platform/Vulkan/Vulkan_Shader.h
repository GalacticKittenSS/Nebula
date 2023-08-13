#pragma once

#include "Nebula/renderer/Shader.h"
#include "Nebula/Maths/Maths.h"

#include <map>
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

		void SetTextureArray(const std::string& name, Ref<Texture> texture) override;
		void SetUniformBuffer(const std::string& name, Ref<UniformBuffer> uniformBuffer) override;
		
		void SetInt(const std::string& name, int value) override;
		void SetIntArray(const std::string& name, int* values, uint32_t count) override;
		void SetFloat(const std::string& name, float value) override;
		void SetFloat2(const std::string& name, const glm::vec2& value) override;
		void SetFloat3(const std::string& name, const glm::vec3& value) override;
		void SetFloat4(const std::string& name, const glm::vec4& value) override;
		void SetMat4(const std::string& name, const glm::mat4& value) override;
		
		static void SetTexture(uint32_t slot, VkDescriptorImageInfo info);
		static void BindPipeline();
	private:
		struct UniformData
		{
			uint32_t descriptorSet = (uint32_t)-1;
			uint32_t binding = (uint32_t)-1;
			uint32_t arrayCount = 0;
			VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
		};
	private:
		std::string ReadFile(const std::string& filepath);
		std::unordered_map<VkShaderStageFlagBits, std::string> PreProcess(const std::string& source);

		void CompileOrGetVulkanBinaries(const std::unordered_map<VkShaderStageFlagBits, std::string>& shaderSources);
		void Reflect(VkShaderStageFlagBits stage, const std::vector<uint32_t>& shaderData);

		VkShaderModule CreateShaderModule(const std::vector<uint32_t>& code);
		void CreatePipeline(VkPipelineShaderStageCreateInfo shaderStages[]);

		UniformData GetUniformFromName(const std::string& name) const;
		UniformData GetUniformFromType(VkDescriptorType type) const;
	private:
		uint32_t m_RendererID;
		std::string m_FilePath;
		std::string m_Name;

		std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>> m_VulkanSPIRV;
		std::unordered_map<VkShaderStageFlagBits, std::vector<char>> m_ShaderCode;

		VkPipeline m_GraphicsPipeline;
		VkPipelineLayout m_PipelineLayout;
		
		std::vector<VkDescriptorSet> m_DescriptorSets;
		std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;

		std::map<std::string, UniformData> m_Uniforms;

		friend class Vulkan_RendererAPI;
		friend class Vulkan_UniformBuffer;
		friend class Vulkan_Texture2D;

		static const Vulkan_Shader* s_BindedInstance;
	};
}