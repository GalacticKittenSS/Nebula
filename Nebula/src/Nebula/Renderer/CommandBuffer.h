#pragma once

#include "Nebula/Core/API.h"
#include "Nebula/Renderer/Image.h"

namespace Nebula
{
	class CommandBuffer
	{
	public:
		virtual ~CommandBuffer() = default;

		virtual void BeginRecording() = 0;
		virtual void EndRecording() = 0;
		virtual bool IsRecording() = 0;
		virtual uint64_t GetCommandBuffer() const = 0;

		static Ref<CommandBuffer> Create();
	};
}