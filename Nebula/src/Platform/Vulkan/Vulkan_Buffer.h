#pragma once

#include "Nebula/renderer/Buffer.h"

#include <vulkan/vulkan.h>

#include "VulkanAPI.h"

namespace Nebula {
	//-----------------------------------------------------//
	/////////////////////////////////////////////////////////
	///////////////////// Vertex BUFFER /////////////////////
	////////////////////////////////////////////////////////
	//-----------------------------------------------------//

	class Vulkan_VertexBuffer : public VertexBuffer 
	{
	public:
		Vulkan_VertexBuffer(uint32_t size);
		Vulkan_VertexBuffer(float* vertices, uint32_t size);
		~Vulkan_VertexBuffer();

		void Bind()   const override;
		void Unbind() const override;
		void SetData(const void* data, uint32_t size = 0) override;
		
		const BufferLayout GetLayout() const override { return m_Layout; }
		void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
	private:
		BufferLayout m_Layout;
		Scope<VulkanBuffer> m_Buffer;
	};

	//----------------------------------------------------//
	////////////////////////////////////////////////////////
	///////////////////// INDEX BUFFER /////////////////////
	////////////////////////////////////////////////////////
	//----------------------------------------------------//

	class Vulkan_IndexBuffer : public IndexBuffer 
	{
	public:
		Vulkan_IndexBuffer(uint32_t* indices, uint32_t count);
		~Vulkan_IndexBuffer();

		void Bind()   const override;
		void Unbind() const override;
		
		uint32_t GetCount() const override { return m_Count; }
	private:
		uint32_t m_Count;
		Scope<VulkanBuffer> m_Buffer;
	};
}