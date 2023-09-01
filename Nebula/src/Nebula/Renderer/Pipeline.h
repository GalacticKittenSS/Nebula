#pragma once

#include "Nebula/Core/API.h"

#include "Shader.h"
#include "RenderPass.h"

namespace Nebula
{
	enum class PipelineShape
	{
		None = 0,
		Triangles,
		Lines
	};

	struct PipelineSpecification
	{
		PipelineShape Shape;
		Ref<Shader> Shader;
		Ref<RenderPass> RenderPass;
		bool DepthTestEnable = true;
	};

	class Pipeline
	{
	public:
		~Pipeline() = default;

		virtual void Bind() const {};

		// Bind Descriptor Sets from linked shader, for all use descriptorSet = -1 (default)
		virtual void BindDescriptorSet(uint32_t descriptorSet = -1) const {};
		virtual void BindDescriptorSet(uint32_t descriptorSet, Ref<UniformBuffer> uniformBuffer) const {};
		
		static Ref<Pipeline> Create(const PipelineSpecification& specification);
	};
}