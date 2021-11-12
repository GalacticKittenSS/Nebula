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
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		NB_ASSERT(status, "Failed to Initialise GLad!");

		NB_INFO("OpenGL Info:");
		NB_INFO("  Vendor: {0}",   glGetString(GL_VENDOR));
		NB_INFO("  Renderer: {0}", glGetString(GL_RENDERER));
		NB_INFO("  Version: {0}",  glGetString(GL_VERSION));
	}

	void OpenGL_Context::SwapBuffers() {
		glfwSwapBuffers(m_WindowHandle);
	}
}