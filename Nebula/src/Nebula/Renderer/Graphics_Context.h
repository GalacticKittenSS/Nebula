#pragma once

#include "Image.h"

namespace Nebula {
	class GraphicsContext {
	public:
		virtual ~GraphicsContext() = default;

		virtual void Init() = 0;
		virtual void Shutdown() = 0;
		
		virtual void SwapBuffers() = 0;
		virtual void SetVsync(bool vsync) = 0;

		virtual uint32_t GetImageIndex() const = 0;
		virtual uint32_t GetImageCount() const = 0;
		virtual const Ref<Image2D> GetImage(uint32_t index = 0) const = 0;
		virtual const void* GetSurface() const = 0;
		
		static Scope<GraphicsContext> Create(void* window);
	};
}
