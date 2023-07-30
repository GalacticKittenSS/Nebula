#include "nbpch.h"
#include "Vulkan_VertexArray.h"

#include "VulkanAPI.h"

namespace Nebula 
{	
	Vulkan_VertexArray::Vulkan_VertexArray() {}
	Vulkan_VertexArray::~Vulkan_VertexArray() {}

	void Vulkan_VertexArray::Bind() const 
	{
		NB_PROFILE_FUNCTION();

		for (auto& buffer : m_VertexBuffers)
			buffer->Bind();

		m_IndexBuffer->Bind();
	}

	void Vulkan_VertexArray::Unbind() const 
	{
		NB_PROFILE_FUNCTION();

		for (auto& buffer : m_VertexBuffers)
			buffer->Unbind();

		m_IndexBuffer->Unbind();
	}

	void Vulkan_VertexArray::AddVertexBuffer(const Ref<VertexBuffer>& buffer) 
	{
		NB_PROFILE_FUNCTION();
		m_VertexBuffers.push_back(buffer);
	}

	void Vulkan_VertexArray::SetIndexBuffer(const Ref<IndexBuffer>& buffer) {
		NB_PROFILE_FUNCTION();

		m_IndexBuffer = buffer;
	}
}