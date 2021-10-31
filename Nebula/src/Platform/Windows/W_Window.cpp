#include "nbpch.h"
#include "W_Window.h"

namespace Nebula {
	static bool s_GLFWInitialized = false;

	Window* Window::Create(const WindowProps& props) {
		return new W_Window(props);
	}

	W_Window::W_Window(const WindowProps& props) {
		Init(props);
	}

	W_Window::~W_Window() {
		ShutDown();
	}

	void W_Window::Init(const WindowProps& props) {
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		NB_INFO("Creating Window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		if (!s_GLFWInitialized) {
			//TODO: glfwTerminate on system shutdown
			int success = glfwInit();
			NB_ASSERT(success, "Could Not Initialise GLFW");

			s_GLFWInitialized = true;
		}

		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
		glfwMakeContextCurrent(m_Window);
		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);
	}

	void W_Window::ShutDown() {
		glfwDestroyWindow(m_Window);
	}

	void W_Window::OnUpdate() {
		glfwPollEvents();
		glfwSwapBuffers(m_Window);
	}

	void W_Window::SetVSync(bool enabled) {
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.Vsync = enabled;
	}

	bool W_Window::IsVSync() const {
		return m_Data.Vsync;
	}
}