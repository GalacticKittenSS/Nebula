#include "nbpch.h"
#include "Vulkan_UniformBuffer.h"

#include "VulkanAPI.h"
#include "Vulkan_Shader.h"
#include "Nebula/Scene/SceneRenderer.h"

namespace Nebula 
{
	std::map<uint32_t, Vulkan_UniformBuffer*> Vulkan_UniformBuffer::s_BindedInstances;

	Vulkan_UniformBuffer::Vulkan_UniformBuffer(uint32_t size, uint32_t binding)
	{
		NB_PROFILE_FUNCTION();

		m_Buffers.resize(2);
		for (uint32_t i = 0; i < m_Buffers.size(); i++)
		{
			m_Buffers[i] = CreateScope<VulkanBuffer>(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}

		Ref<Vulkan_Shader> shader = std::static_pointer_cast<Vulkan_Shader>(SceneRenderer::GetShader());

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_Buffers[0]->GetBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = 192;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = shader->m_DescriptorSet;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(VulkanAPI::GetDevice(), 1, &descriptorWrite, 0, nullptr);
		
		s_BindedInstances[binding] = this;
	}

	Vulkan_UniformBuffer::~Vulkan_UniformBuffer()
	{
		NB_PROFILE_FUNCTION();
	}


	void Vulkan_UniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		NB_PROFILE_FUNCTION();

		m_Buffers[0]->SetData(data, size, offset);
	}

}