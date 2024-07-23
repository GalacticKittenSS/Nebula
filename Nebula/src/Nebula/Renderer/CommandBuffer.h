#pragma once

#include "Nebula/Core/API.h"
#include "Nebula/Renderer/Image.h"

namespace Nebula
{
	class CommandBuffer
	{
	public:
		virtual ~CommandBuffer() = default;

		virtual void BeginRecording() {};
		virtual void EndRecording() {};
		virtual bool IsRecording() const { return false; };
		virtual uint64_t GetCommandBuffer() const { return -1; };

		static Ref<CommandBuffer> Create();
	};
}