#include "nbpch.h"
#include "Vulkan_Buffer.h"

#include "VulkanAPI.h"
#include "Nebula/Renderer/Render_Command.h"

namespace Nebula
{
	//-----------------------------------------------------//
	/////////////////////////////////////////////////////////
	///////////////////// Vertex BUFFER /////////////////////
	////////////////////////////////////////////////////////
	//-----------------------------------------------------//

	Vulkan_VertexBuffer::Vulkan_VertexBuffer(uint32_t size) 
	{
		NB_PROFILE_FUNCTION();

		m_Buffer = CreateScope<VulkanBuffer>(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	}

	Vulkan_VertexBuffer::Vulkan_VertexBuffer(float* vertices, uint32_t size) 
	{
		NB_PROFILE_FUNCTION();

		m_Buffer = CreateScope<VulkanBuffer>(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		SetData(vertices, size);
	}

	Vulkan_VertexBuffer::~Vulkan_VertexBuffer() 
	{
		NB_PROFILE_FUNCTION();
	}

	void Vulkan_VertexBuffer::Bind() const {
		NB_PROFILE_FUNCTION();

		VkCommandBuffer commandBuffer = VulkanAPI::GetCommandBuffer();

		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_Buffer->GetBuffer(), &offset);
	}

	void Vulkan_VertexBuffer::Unbind() const {
		NB_PROFILE_FUNCTION();
	}

	void Vulkan_VertexBuffer::SetData(const void* data, uint32_t size) 
	{
		NB_PROFILE_FUNCTION();

		m_Buffer->SetData(data, size);
	}

	//----------------------------------------------------//
	////////////////////////////////////////////////////////
	///////////////////// INDEX BUFFER /////////////////////
	////////////////////////////////////////////////////////
	//----------------------------------------------------//

	Vulkan_IndexBuffer::Vulkan_IndexBuffer(uint32_t* indices, uint32_t count) : m_Count(count) 
	{
		NB_PROFILE_FUNCTION();

		uint32_t size = count * sizeof(uint32_t);
		m_Buffer = CreateScope<VulkanBuffer>(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		m_Buffer->SetData(indices, size);
	}

	Vulkan_IndexBuffer::~Vulkan_IndexBuffer() 
	{
		NB_PROFILE_FUNCTION();
	}

	void Vulkan_IndexBuffer::Bind() const {
		NB_PROFILE_FUNCTION();

		VkCommandBuffer commandBuffer = VulkanAPI::GetCommandBuffer();
		vkCmdBindIndexBuffer(commandBuffer, m_Buffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void Vulkan_IndexBuffer::Unbind() const {
		NB_PROFILE_FUNCTION();

		VkCommandBuffer commandBuffer = VulkanAPI::GetCommandBuffer();
		vkCmdBindIndexBuffer(commandBuffer, VK_NULL_HANDLE, 0, VK_INDEX_TYPE_UINT32);
	}

}