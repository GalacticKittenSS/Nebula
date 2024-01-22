#pragma once

#include <Nebula/Core/API.h>

namespace Nebula
{
	class CommandBuffer
	{
	public:
		virtual void BeginRecording() = 0;
		virtual void EndRecording() = 0;
		virtual void Submit() = 0;
		virtual bool IsRecording() = 0;

		static Ref<CommandBuffer> Create();
	};
}