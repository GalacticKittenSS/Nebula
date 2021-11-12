#pragma once

#include "Nebula/renderer/Graphics_Context.h"

struct GLFWwindow;

namespace Nebula {
	class OpenGL_Context : public GraphicsContext {
	public:
		OpenGL_Context(GLFWwindow* windowHandle);

		void Init() override;
		void SwapBuffers() override;
	private:
		GLFWwindow* m_WindowHandle;
	};
}