#include "nbpch.h"
#include "Vulkan_VertexArray.h"

#include <vulkan/vulkan.h>

namespace Nebula 
{
	namespace Utils
	{
		static VkFormat ShaderTypetoVulkan(ShaderDataType type) {
			switch (type) {
			case ShaderDataType::Float:		return VK_FORMAT_R32_SFLOAT;
			case ShaderDataType::Float2:	return VK_FORMAT_R32G32_SFLOAT;
			case ShaderDataType::Float3:	return VK_FORMAT_R32G32B32_SFLOAT;
			case ShaderDataType::Float4:	return VK_FORMAT_R32G32B32A32_SFLOAT;
			case ShaderDataType::Int:		return VK_FORMAT_R32_SINT;
			case ShaderDataType::Int2:		return VK_FORMAT_R32G32_SINT;
			case ShaderDataType::Int3:		return VK_FORMAT_R32G32B32_SINT;
			case ShaderDataType::Int4:		return VK_FORMAT_R32G32B32A32_SINT;
			case ShaderDataType::Mat3:		return VK_FORMAT_R32G32B32_SFLOAT;
			case ShaderDataType::Mat4:		return VK_FORMAT_R32G32B32A32_SFLOAT;
			case ShaderDataType::Bool:		return VK_FORMAT_R16_SFLOAT;
			}

			return VK_FORMAT_R32_SFLOAT;
		}
	}
	
	Vulkan_VertexArray::Vulkan_VertexArray() {
		NB_PROFILE_FUNCTION();
	}

	Vulkan_VertexArray::~Vulkan_VertexArray() {
		NB_PROFILE_FUNCTION();
	}

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

		const auto& layout = buffer->GetLayout();
		std::vector<VkVertexInputAttributeDescription> attribDescriptions(layout.GetElements().size());
		
		for (auto& element : layout) 
		{
			VkVertexInputAttributeDescription attribDescription{};
			attribDescription.binding = 0;
			attribDescription.location = m_VertexBufferIndex;
			attribDescription.format = Utils::ShaderTypetoVulkan(element.Type);
			attribDescription.offset = element.Offset;
			m_VertexBufferIndex++;

			attribDescriptions.push_back(attribDescription);
		}

		VkVertexInputBindingDescription bindingDescription;
		bindingDescription.binding = 0;
		bindingDescription.stride = layout.GetStride();
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		m_VertexBuffers.push_back(buffer);
	}

	void Vulkan_VertexArray::SetIndexBuffer(const Ref<IndexBuffer>& buffer) {
		NB_PROFILE_FUNCTION();

		m_IndexBuffer = buffer;
	}
}