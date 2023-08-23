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
		const void* GetSurface() const override { return nullptr; }
	private:
		GLFWwindow* m_WindowHandle;
	};
}