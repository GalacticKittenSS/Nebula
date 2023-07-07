#pragma once

namespace Nebula {
	class GraphicsContext {
	public:
		virtual ~GraphicsContext() = default;

		virtual void Init() = 0;
		virtual void Shutdown() = 0;
		virtual void SwapBuffers() = 0;
		virtual void* GetSurface() = 0;

		static Scope<GraphicsContext> Create(void* window);
	};
}
