#include "nbpch.h"
#include "OpenGL_Context.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/GL.h>

namespace Nebula {
	OpenGL_Context::OpenGL_Context(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle) {
		NB_ASSERT(windowHandle, "Window Handle is NULL!")
	}

	void OpenGL_Context::Init() {
		NB_PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		NB_ASSERT(status, "Failed to Initialise GLad!");

		NB_INFO("OpenGL Info:");
		NB_INFO("  Vendor: {0}",   glGetString(GL_VENDOR));
		NB_INFO("  Renderer: {0}", glGetString(GL_RENDERER));
		NB_INFO("  Version: {0}",  glGetString(GL_VERSION));

#ifdef NB_ENABLE_ASSERTS
		int versionMajor;
		int versionMinor;
		glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
		glGetIntegerv(GL_MINOR_VERSION, &versionMinor);

		NB_ASSERT(versionMajor > 4 || (versionMajor == 4 && versionMinor >= 5), "Nebula Requires OpenGL version 4.5 and Up!");
#endif
	}

	void OpenGL_Context::SwapBuffers() {
		NB_PROFILE_FUNCTION();

		glfwSwapBuffers(m_WindowHandle);
	}
}