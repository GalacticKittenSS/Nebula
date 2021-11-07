#pragma once

#include "Nebula/renderer/Graphics_Context.h"

struct GLFWwindow;

namespace Nebula {
	class OpenGLContext : public GraphicsContext {
	public:
		OpenGLContext(GLFWwindow* windowHandle);

		void Init() override;
		void SwapBuffers() override;
	private:
		GLFWwindow* m_WindowHandle;
	};
}