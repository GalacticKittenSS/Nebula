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
		Ref<Shader> Shader;
		Ref<RenderPass> RenderPass;
		PipelineShape Shape;
		
		bool BackfaceCulling = true;
		bool DepthTest = true;
		bool DepthWrite = true;
		float LineWidth = 1.0f;

		//Debug
		std::string DebugName;
	};

	class Pipeline
	{
	public:
		~Pipeline() = default;

		virtual void Bind() const {};

		// Bind Descriptor Sets from linked shader, for all use descriptorSet = -1 (default)
		virtual void BindDescriptorSet(uint32_t descriptorSet = -1) const {};
		virtual void BindDescriptorSet(uint32_t descriptorSet, Ref<UniformBuffer> uniformBuffer) const {};
		virtual void BindDescriptorSet(Ref<DescriptorSet> descriptorSets) const {};

		static Ref<Pipeline> Create(const PipelineSpecification& specification);
	};
}