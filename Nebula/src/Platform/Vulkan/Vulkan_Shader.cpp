#include "nbpch.h"
#include "Vulkan_Shader.h"

#include "Nebula/Utils/Time.h"
#include "Nebula/Scene/SceneRenderer.h"
#include "Nebula/Renderer/Render_Command.h"
#include "Nebula/Core/Application.h"

#include "VulkanAPI.h"
#include "Vulkan_Context.h"
#include "Vulkan_Framebuffer.h"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace Nebula
{
	namespace Utils {
		static VkShaderStageFlagBits ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
				return VK_SHADER_STAGE_VERTEX_BIT;
			if (type == "fragment" || type == "pixel")
				return VK_SHADER_STAGE_FRAGMENT_BIT;

			NB_ASSERT(false, "Unknown shader type!");
			return VK_SHADER_STAGE_VERTEX_BIT;
		}

		static const char* GetCacheDirectory()
		{
			// TODO: make sure the assets directory is valid
			return "Resources/cache/shader/vulkan/";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			std::string cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
				std::filesystem::create_directories(cacheDirectory);
		}

		static shaderc_shader_kind VKShaderStageToShaderC(VkShaderStageFlagBits stage)
		{
			switch (stage)
			{
			case VK_SHADER_STAGE_VERTEX_BIT:   return shaderc_glsl_vertex_shader;
			case VK_SHADER_STAGE_FRAGMENT_BIT: return shaderc_glsl_fragment_shader;
			}

			assert(false);
			return (shaderc_shader_kind)0;
		}

		static const char* VKShaderStageCachedVulkanFileExtension(uint32_t stage)
		{
			switch (stage)
			{
			case VK_SHADER_STAGE_VERTEX_BIT:    return ".cached_vulkan.vert";
			case VK_SHADER_STAGE_FRAGMENT_BIT:  return ".cached_vulkan.frag";
			}

			assert(false);
			return "";
		}

		static const char* VKShaderStageToString(VkShaderStageFlagBits stage)
		{
			switch (stage)
			{
			case VK_SHADER_STAGE_VERTEX_BIT:   return "VK_SHADER_STAGE_VERTEX_SHADER";
			case VK_SHADER_STAGE_FRAGMENT_BIT: return "VK_SHADER_STAGE_FRAGMENT_SHADER";
			}

			assert(false);
			return nullptr;
		}
		
		VkFormat SPIRTypeToVulkan(spirv_cross::SPIRType type, uint32_t& elementSize)
		{
#define VulkanSwitchFormat(vecsize, size, type)\
	switch (vecsize) {\
		case 1: return VK_FORMAT_R##size##_##type;\
		case 2: return VK_FORMAT_R##size##G##size##_##type;\
		case 3: return VK_FORMAT_R##size##G##size##B##size##_##type;\
		case 4: return VK_FORMAT_R##size##G##size##B##size##A##size##_##type;\
	}\
	NB_ASSERT(false); return VK_FORMAT_R##size##_##type;

			switch (type.basetype)
			{
			case spirv_cross::SPIRType::Short:	elementSize = sizeof(short); VulkanSwitchFormat(type.vecsize, 16, SINT);
			case spirv_cross::SPIRType::UShort:	elementSize = sizeof(short); VulkanSwitchFormat(type.vecsize, 16, UINT);
			case spirv_cross::SPIRType::Int:	elementSize = sizeof(int);	VulkanSwitchFormat(type.vecsize, 32, SINT);
			case spirv_cross::SPIRType::UInt:	elementSize = sizeof(int);	VulkanSwitchFormat(type.vecsize, 32, UINT);
			case spirv_cross::SPIRType::Float:	elementSize = sizeof(float); VulkanSwitchFormat(type.vecsize, 32, SFLOAT);
			}


			NB_ASSERT(false);
			return VK_FORMAT_R16_SFLOAT;
		}

		VkDescriptorSetLayoutBinding AddLayoutBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlagBits stage)
		{
			VkDescriptorSetLayoutBinding layoutBinding{};
			layoutBinding.binding = binding;
			layoutBinding.descriptorCount = 1;
			layoutBinding.descriptorType = type;
			layoutBinding.stageFlags = stage;
			return layoutBinding;
		}

		void AddResourceLayoutBinding(std::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings, const spirv_cross::Compiler& compiler, VkShaderStageFlagBits stage)
		{
			spirv_cross::ShaderResources resources = compiler.get_shader_resources();
			
			for (const auto& resource : resources.uniform_buffers)
			{
				uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
				VkDescriptorSetLayoutBinding layoutBinding = AddLayoutBinding(binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stage);
				descriptorSetLayoutBindings.push_back(layoutBinding);
			}

			for (const auto& resource : resources.sampled_images)
			{
				uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
				VkDescriptorSetLayoutBinding layoutBinding = AddLayoutBinding(binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage);
				descriptorSetLayoutBindings.push_back(layoutBinding);
			}
		}
	}


	Vulkan_Shader::Vulkan_Shader(const std::string& filepath)
		: m_FilePath(filepath)
	{
		NB_PROFILE_FUNCTION();

		Utils::CreateCacheDirectoryIfNeeded();

		std::string source = ReadFile(filepath);
		auto shaderSources = PreProcess(source);

		{
			Timer timer;
			CompileOrGetVulkanBinaries(shaderSources);
			NB_WARN("Shader creation took {0} ms", timer.Elapsed() * 1000);
		}

		// Extract name from filepath
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		m_Name = filepath.substr(lastSlash, count);

		VkShaderModule vertShaderModule = CreateShaderModule(m_VulkanSPIRV[VK_SHADER_STAGE_VERTEX_BIT]);
		VkShaderModule fragShaderModule = CreateShaderModule(m_VulkanSPIRV[VK_SHADER_STAGE_FRAGMENT_BIT]);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";


		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
		
		// Descriptor Set Layout
		{
			spirv_cross::Compiler vertexCompiler(m_VulkanSPIRV[VK_SHADER_STAGE_VERTEX_BIT]);
			spirv_cross::Compiler fragmentCompiler(m_VulkanSPIRV[VK_SHADER_STAGE_FRAGMENT_BIT]);
			
			std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
			Utils::AddResourceLayoutBinding(descriptorSetLayoutBindings, vertexCompiler, VK_SHADER_STAGE_VERTEX_BIT);
			Utils::AddResourceLayoutBinding(descriptorSetLayoutBindings, fragmentCompiler, VK_SHADER_STAGE_FRAGMENT_BIT);

			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = (uint32_t)descriptorSetLayoutBindings.size();
			layoutInfo.pBindings = descriptorSetLayoutBindings.data();

			VkResult result = vkCreateDescriptorSetLayout(VulkanAPI::GetDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout);
			NB_ASSERT(result == VK_SUCCESS, "Failed to create descriptor set layout!");
		}
		
		CreatePipeline(shaderStages);

		// Descriptor Set Pool
		{
			std::vector<VkDescriptorPoolSize> pool_sizes = {
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};

			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = (uint32_t)pool_sizes.size();
			poolInfo.pPoolSizes = pool_sizes.data();
			poolInfo.maxSets = 1000;

			VkResult result = vkCreateDescriptorPool(VulkanAPI::GetDevice(), &poolInfo, nullptr, &m_DescriptorPool);
			NB_ASSERT(result == VK_SUCCESS, "Failed to create descriptor pool!");
		}

		// Descriptor Set
		{
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = m_DescriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &m_DescriptorSetLayout;

			VkResult result = vkAllocateDescriptorSets(VulkanAPI::GetDevice(), &allocInfo, &m_DescriptorSet);
			NB_ASSERT(result == VK_SUCCESS, "Failed to allocate descriptor sets!");
		}

		vkDestroyShaderModule(VulkanAPI::GetDevice(), vertShaderModule, nullptr);
		vkDestroyShaderModule(VulkanAPI::GetDevice(), fragShaderModule, nullptr);
	}

	Vulkan_Shader::Vulkan_Shader(const std::string& name, const std::string& vertSrc, const std::string& fragSrc)
		: m_Name(name)
	{
		
	}

	Vulkan_Shader::~Vulkan_Shader()
	{
		vkDestroyPipeline(VulkanAPI::GetDevice(), m_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(VulkanAPI::GetDevice(), m_PipelineLayout, nullptr);

		vkDestroyDescriptorPool(VulkanAPI::GetDevice(), m_DescriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(VulkanAPI::GetDevice(), m_DescriptorSetLayout, nullptr);
	}

	void Vulkan_Shader::Bind() const
	{

	}
	
	void Vulkan_Shader::Unbind() const
	{

	}

	std::string Vulkan_Shader::ReadFile(const std::string& filepath) {
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
		if (in) {
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1) {
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
			}
			else
			{
				NB_ERROR("Could not read from file '{0}'", filepath);
			}
		}
		else
		{
			NB_ERROR("Could not open file '{0}'", filepath);
		}

		return result;
	}

	std::unordered_map<VkShaderStageFlagBits, std::string> Vulkan_Shader::PreProcess(const std::string& source) {
		std::unordered_map<VkShaderStageFlagBits, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0); //Start of shader type declaration line
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos); //End of shader type declaration line
			NB_ASSERT(eol != std::string::npos && "Syntax error");
			size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
			std::string type = source.substr(begin, eol - begin);
			NB_ASSERT(Utils::ShaderTypeFromString(type) && "Invalid shader type specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
			NB_ASSERT(nextLinePos != std::string::npos && "Syntax error");
			pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line

			shaderSources[Utils::ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
		}

		return shaderSources;
	}

	void Vulkan_Shader::CompileOrGetVulkanBinaries(const std::unordered_map<VkShaderStageFlagBits, std::string>& shaderSources)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		const bool optimize = true;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();
		
		auto& shaderData = m_VulkanSPIRV;
		shaderData.clear();
		for (auto&& [stage, source] : shaderSources)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::VKShaderStageCachedVulkanFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open()) {
				in.seekg(0, std::ios::end);
				size_t size = static_cast<size_t>(in.tellg());
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));

				in.seekg(0);
				in.read((char*)data.data(), size);
				in.close();

				continue;
			}
			
			shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::VKShaderStageToShaderC(stage), m_FilePath.c_str(), options);
			if (module.GetCompilationStatus() != shaderc_compilation_status_success)
			{
				NB_ERROR(module.GetErrorMessage());
				NB_ASSERT(false);
			}

			shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

			std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
			if (out.is_open())
			{
				auto& data = shaderData[stage];
				out.write((char*)data.data(), data.size() * sizeof(uint32_t));
				out.flush();
				out.close();
			}
		}

		for (auto&& [stage, data] : shaderData)
			Reflect(stage, data);
	}

	void Vulkan_Shader::Reflect(VkShaderStageFlagBits stage, const std::vector<uint32_t>& shaderData)
	{
		spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		NB_TRACE("Vulkan_Shader::Reflect - {0} {1}", Utils::VKShaderStageToString(stage), m_FilePath);
		NB_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
		NB_TRACE("    {0} resources", resources.sampled_images.size());

		NB_TRACE("Uniform buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t bufferSize = (uint32_t)compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			int memberCount = (int)bufferType.member_types.size();

			NB_TRACE("  {0}", resource.name);
			NB_TRACE("    Size = {0}", bufferSize);
			NB_TRACE("    Binding = {0}", binding);
			NB_TRACE("    Members = {0}", memberCount);
		}
	}

	VkShaderModule Vulkan_Shader::CreateShaderModule(const std::vector<uint32_t>& code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size() * sizeof(uint32_t);
		createInfo.pCode = code.data();

		VkShaderModule shaderModule;
		VkResult result = vkCreateShaderModule(VulkanAPI::GetDevice(), &createInfo, nullptr, &shaderModule);
		if (result != VK_SUCCESS)
		{
			NB_ERROR("Failed to create shader module!");
		}

		return shaderModule;
	}

	void Vulkan_Shader::CreatePipeline(VkPipelineShaderStageCreateInfo shaderStages[])
	{
		// Pipeline Layout
		{
			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = 1;
			pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;

			VkResult result = vkCreatePipelineLayout(VulkanAPI::GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout);
			NB_ASSERT(result == VK_SUCCESS, "Failed to create pipeline layout");
		}

		spirv_cross::Compiler vertexCompiler(m_VulkanSPIRV[VK_SHADER_STAGE_VERTEX_BIT]);
		spirv_cross::ShaderResources vertexResources = vertexCompiler.get_shader_resources();
		
		size_t inputsSize = vertexResources.stage_inputs.size();
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(inputsSize);

		uint32_t offset = 0;
		for (uint32_t i = 0; i < inputsSize; i++)
		{
			const auto& resource = vertexResources.stage_inputs[i];
			const auto& type = vertexCompiler.get_type(resource.type_id);
			uint32_t elementSize;

			attributeDescriptions[i].binding = vertexCompiler.get_decoration(resource.id, spv::DecorationBinding);
			attributeDescriptions[i].location = vertexCompiler.get_decoration(resource.id, spv::DecorationLocation);
			attributeDescriptions[i].format = Utils::SPIRTypeToVulkan(type, elementSize);
			attributeDescriptions[i].offset = offset;
			offset += type.vecsize * elementSize;
		}

		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = offset;
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		spirv_cross::Compiler fragmentCompiler(m_VulkanSPIRV[VK_SHADER_STAGE_FRAGMENT_BIT]);
		spirv_cross::ShaderResources fragmentResources = fragmentCompiler.get_shader_resources();

		size_t outputCount = fragmentResources.stage_outputs.size();
		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(outputCount);

		for (uint32_t i = 0; i < outputCount; i++)
		{
			colorBlendAttachments[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachments[i].blendEnable = VK_FALSE;
		}
		
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = (uint32_t)colorBlendAttachments.size();
		colorBlending.pAttachments = colorBlendAttachments.data();
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;
		
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = m_PipelineLayout;
		pipelineInfo.renderPass = Vulkan_FrameBuffer::s_BindedInstance->m_RenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		
		VkResult result = vkCreateGraphicsPipelines(VulkanAPI::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline);
		NB_ASSERT(result == VK_SUCCESS, "Failed to create graphics pipeline!");
	}

	void Vulkan_Shader::SetInt(const std::string& name, int value)
	{
	}

	void Vulkan_Shader::SetIntArray(const std::string& name, int* values, uint32_t count)
	{
	}

	void Vulkan_Shader::SetFloat(const std::string& name, float value)
	{
	}

	void Vulkan_Shader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
	}

	void Vulkan_Shader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
	}

	void Vulkan_Shader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
	}

	void Vulkan_Shader::SetMat4(const std::string& name, const glm::mat4& value)
	{
	}

	void Vulkan_Shader::UploadUniformInt(const std::string& name, const int value)
	{
	}

	void Vulkan_Shader::UploadUniformIntArray(const std::string& name, int* values, uint32_t count)
	{
	}

	void Vulkan_Shader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
	}

	void Vulkan_Shader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
	}

	void Vulkan_Shader::UploadUniformFloat(const std::string& name, const float values)
	{
	}

	void Vulkan_Shader::UploadUniformFloat2(const std::string& name, const glm::vec2& values)
	{
	}

	void Vulkan_Shader::UploadUniformFloat3(const std::string& name, const glm::vec3& values)
	{
	}
	
	void Vulkan_Shader::UploadUniformFloat4(const std::string& name, const glm::vec4& values)
	{
	}
}