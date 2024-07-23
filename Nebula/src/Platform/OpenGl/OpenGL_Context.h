#pragma once

#include "Nebula/renderer/Graphics_Context.h"

struct GLFWwindow;

namespace Nebula {
	class OpenGL_Context : public GraphicsContext {
	public:
		OpenGL_Context(GLFWwindow* windowHandle);

		void Init() override;
		void Shutdown() override;

		void SwapBuffers() override;
		void SetVsync(bool vsync) override;

		uint32_t GetImageIndex() const override { return 0; };
		uint32_t GetImageCount() const override { return 1; };
		const Ref<Image2D> GetImage(uint32_t index = 0) const override { return nullptr; }
		
		const void* GetSurface() const override { return nullptr; }
	private:
		GLFWwindow* m_WindowHandle;
	};
}