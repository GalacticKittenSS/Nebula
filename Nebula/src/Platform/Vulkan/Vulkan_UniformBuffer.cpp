#include "nbpch.h"
#include "Vulkan_UniformBuffer.h"

#include "VulkanAPI.h"
#include "Vulkan_Pipeline.h"
#include "Nebula/Scene/SceneRenderer.h"

namespace Nebula 
{
	Vulkan_UniformBuffer::Vulkan_UniformBuffer(uint32_t size, uint32_t binding)
		: m_Binding(binding)
	{
		NB_PROFILE_FUNCTION();

		m_Buffer = CreateScope<VulkanBuffer>(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorCount = 1u;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &layoutBinding;

		VkResult result = vkCreateDescriptorSetLayout(VulkanAPI::GetDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout);
		NB_ASSERT(result == VK_SUCCESS, "Failed to create descriptor set layout!");

		VulkanAPI::AllocateDescriptorSet(m_DescriptorSet, m_DescriptorSetLayout);

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_Buffer->GetBuffer();
		bufferInfo.range = m_Buffer->GetSize();
		bufferInfo.offset = 0;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_DescriptorSet;
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(VulkanAPI::GetDevice(), 1, &descriptorWrite, 0, nullptr);
	}

	Vulkan_UniformBuffer::~Vulkan_UniformBuffer()
	{
		NB_PROFILE_FUNCTION();

		VulkanAPI::SubmitResource([descriptorSet = m_DescriptorSet]() {
			vkFreeDescriptorSets(VulkanAPI::GetDevice(), VulkanAPI::s_DescriptorPool, 1, &descriptorSet);
		});
	}

	void Vulkan_UniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		NB_PROFILE_FUNCTION();

		m_Buffer->SetData(data, size, offset);
	}
}