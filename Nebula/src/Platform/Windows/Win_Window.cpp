#include "nbpch.h"
#include "Win_Window.h"

#include "Nebula/Core/Input.h"

#include "Nebula/events/Window_Event.h"
#include "Nebula/events/Mouse_Event.h"
#include "Nebula/events/Key_Event.h"

#include "Nebula/Renderer/Renderer.h"

#include "Platform/OpenGl/OpenGL_Context.h"

namespace Nebula {
	static uint8_t s_GLFWWindowCount = 0;

	static void GLFWErrorCallback(int error, const char* description) {
		NB_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	Win_Window::Win_Window(const WindowProps& props) {
		Init(props);
	}

	Win_Window::~Win_Window() {
		ShutDown();
	}

	void Win_Window::Init(const WindowProps& props) {
		NB_PROFILE_FUNCTION();

		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		NB_INFO("Creating Window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		if (s_GLFWWindowCount == 0) {
			//TODO: glfwTerminate on system shutdown
			NB_INFO("Initializing GLFW");
			int success = glfwInit();
			NB_ASSERT(success, "Could Not Initialise GLFW");
			glfwSetErrorCallback(GLFWErrorCallback);
		}

#ifdef NB_DEBUG
		if (Renderer::GetAPI() == RendererAPI::API::OpenGL)
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
		++s_GLFWWindowCount;
		m_Context = GraphicsContext::Create(m_Window);

		m_Context->Init();

		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		//Set GLFW Callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});
		
		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			switch (action) {
				case GLFW_PRESS: {
					KeyPressedEvent event(static_cast<KeyCode>(key), 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE: {
					KeyReleasedEvent event(static_cast<KeyCode>(key));
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT: {
					KeyPressedEvent event(static_cast<KeyCode>(key), 1);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int character) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			KeyTypedEvent event(static_cast<KeyCode>(character));
			data.EventCallback(event);
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action) {
				case GLFW_PRESS: {
					MouseButtonPressedEvent event(static_cast<MouseCode>(button));
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE: {
					MouseButtonReleasedEvent event(static_cast<MouseCode>(button));
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});
	}

	void Win_Window::ShutDown() {
		NB_PROFILE_FUNCTION();

		glfwDestroyWindow(m_Window);
		--s_GLFWWindowCount;

		if (s_GLFWWindowCount == 0) {
			NB_INFO("Terminating GLFW");
			glfwTerminate();
		}
	}

	void Win_Window::Update() {
		NB_PROFILE_FUNCTION();
		
		glfwPollEvents();
		m_Context->SwapBuffers();
	}

	void Win_Window::SetVSync(bool enabled) {
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.Vsync = enabled;
	}

	bool Win_Window::IsVSync() const {
		return m_Data.Vsync;
	}

	void Win_Window::LockAspectRatio(uint8_t width, uint8_t height) {
		glfwSetWindowAspectRatio(m_Window, width, height);
	}

	void Win_Window::UnlockAspectRatio() {
		glfwSetWindowAspectRatio(m_Window, GLFW_DONT_CARE, GLFW_DONT_CARE);
	}
}