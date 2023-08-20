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

		virtual void Bind() const = 0;

		static Ref<Pipeline> Create(const PipelineSpecification& specification);
	};
}