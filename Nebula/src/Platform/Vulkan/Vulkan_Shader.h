#pragma once

#include "Nebula/renderer/Shader.h"
#include "Nebula/Maths/Maths.h"

#include <map>
#include <unordered_map>
#include <vulkan/vulkan.h>

namespace Nebula {
	struct UniformData
	{
		uint32_t DescriptorSet = (uint32_t)-1;
		uint32_t Binding = (uint32_t)-1;
		uint32_t ArrayCount = 0;
		VkDescriptorType Type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
	};

	class Vulkan_Shader : public Shader {
	public:
		Vulkan_Shader(const std::string& path);
		Vulkan_Shader(const std::string& name, const std::string& vertSrc, const std::string& fragSrc);
		~Vulkan_Shader();

		void Bind() const override;
		void Unbind() const override;

		const std::string& GetName() const override { return m_Name; }

		Ref<DescriptorSet> AllocateDescriptorSets() const;

		void SetInt(const std::string& name, int value) override;
		void SetIntArray(const std::string& name, int* values, uint32_t count) override;
		void SetFloat(const std::string& name, float value) override;
		void SetFloat2(const std::string& name, const glm::vec2& value) override;
		void SetFloat3(const std::string& name, const glm::vec3& value) override;
		void SetFloat4(const std::string& name, const glm::vec4& value) override;
		void SetMat4(const std::string& name, const glm::mat4& value) override;

		size_t GetFragmentOutputCount() const override;

		void GetVulkanVertexInputInfo(std::vector<VkVertexInputAttributeDescription>& attributeDescriptions, uint32_t& offset) const;
		inline const std::array<VkPipelineShaderStageCreateInfo, 2>& GetVulkanShaderStages() const { return m_ShaderStages; }
		inline const std::vector<VkDescriptorSetLayout>& GetVulkanDescriptorSetLayouts() const { return m_DescriptorSetLayouts; }
		inline const std::vector<VkDescriptorSet>& GetVulkanDescriptorSets() const { return {}; }//m_DescriptorSets;}

		UniformData GetUniformFromName(const std::string& name) const;
	private:
		std::string ReadFile(const std::string& filepath);
		std::unordered_map<VkShaderStageFlagBits, std::string> PreProcess(const std::string& source);

		void CompileOrGetVulkanBinaries(const std::unordered_map<VkShaderStageFlagBits, std::string>& shaderSources);
		void Reflect(VkShaderStageFlagBits stage, const std::vector<uint32_t>& shaderData);

		VkShaderModule CreateShaderModule(const std::vector<uint32_t>& code);
	private:
		uint32_t m_RendererID;
		std::string m_FilePath;
		std::string m_Name;

		std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>> m_VulkanSPIRV;
		std::unordered_map<VkShaderStageFlagBits, std::vector<char>> m_ShaderCode;

		std::array<VkPipelineShaderStageCreateInfo, 2> m_ShaderStages;

		VkPipelineLayout m_PipelineLayout;
		
		std::map<std::string, UniformData> m_Uniforms;
		std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;
	};

	class Vulkan_DescriptorSet : public DescriptorSet
	{
	public:
		Vulkan_DescriptorSet(const Vulkan_Shader* shader, std::vector<VkDescriptorSetLayout> layouts);

		void SetResource(const std::string& uniformName, Ref<UniformBuffer> uniformBuffer) override;
		void SetResource(const std::string& uniformName, Ref<Texture2D> texture, uint32_t slot) override;

		const std::vector<VkDescriptorSet>& GetDescriptorSets() const { return m_DescriptorSets; }
	private:
		std::vector<VkDescriptorSet> m_DescriptorSets;
		const Vulkan_Shader* m_Shader = nullptr;
	};
}