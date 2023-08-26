#include "nbpch.h"
#include "Vulkan_UniformBuffer.h"

#include "VulkanAPI.h"
#include "Vulkan_Shader.h"
#include "Nebula/Scene/SceneRenderer.h"

namespace Nebula 
{
	Vulkan_UniformBuffer::Vulkan_UniformBuffer(uint32_t size, uint32_t binding)
		: m_Binding(binding)
	{
		NB_PROFILE_FUNCTION();

		m_Buffers.resize(2);
		for (uint32_t i = 0; i < m_Buffers.size(); i++)
		{
			m_Buffers[i] = CreateScope<VulkanBuffer>(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		}
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