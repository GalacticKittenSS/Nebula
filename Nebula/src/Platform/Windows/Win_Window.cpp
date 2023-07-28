#include "nbpch.h"
#include "Win_Window.h"

#include "Nebula/Core/Input.h"
#include "Nebula/Maths/MinMax.h"

#include "Nebula/events/Window_Event.h"
#include "Nebula/events/Mouse_Event.h"
#include "Nebula/events/Key_Event.h"

#include "Nebula/Renderer/Renderer.h"

#include "Platform/OpenGl/OpenGL_Context.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <stb_image.h>

namespace Nebula {
	static uint8_t s_GLFWWindowCount = 0;
	static bool s_GLFWInit = false;

	static void GLFWErrorCallback(int error, const char* description) {
		NB_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	Win_Window::Win_Window(const WindowProps& props) {
		Init(props);
	}

	Win_Window::~Win_Window() {
		ShutDown();
	}

#ifdef NB_WINDOWS // Static Functions
	void Window::InitAPI()
	{
		NB_ASSERT(!s_GLFWInit, "GLFW has already been initialized!");

		NB_INFO("Initializing GLFW");
		int success = glfwInit();
		NB_ASSERT(success, "Could Not Initialise GLFW");
		glfwSetErrorCallback(GLFWErrorCallback);

		s_GLFWInit = true;
	}

	void Window::ShutdownAPI()
	{
		NB_INFO("Terminating GLFW");
		glfwTerminate();
	}

	const char** Window::GetExtensions(uint32_t& count)
	{
		return glfwGetRequiredInstanceExtensions(&count);
	}
#endif // NB_WINDOWS

	void Win_Window::Init(const WindowProps& props) {
		NB_PROFILE_FUNCTION();

		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		NB_INFO("Creating Window {0} ({1}, {2})", props.Title, props.Width, props.Height);

#ifdef NB_DEBUG
		if (Renderer::GetAPI() == RendererAPI::API::OpenGL)
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

		if (Renderer::GetAPI() == RendererAPI::API::Vulkan)
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

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
					KeyPressedEvent event(static_cast<KeyCode>(key));
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE: {
					KeyReleasedEvent event(static_cast<KeyCode>(key));
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT: {
					KeyPressedEvent event(static_cast<KeyCode>(key), true);
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

		glfwSetDropCallback(m_Window, [](GLFWwindow* window, int pathCount, const char* paths[])
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			std::vector<std::filesystem::path> filepaths(pathCount);
			for (int i = 0; i < pathCount; i++)
				filepaths[i] = paths[i];

			WindowDropEvent event(std::move(filepaths));
			data.EventCallback(event);
		});
	}

	void Win_Window::ShutDown() {
		NB_PROFILE_FUNCTION();

		m_Context->Shutdown();

		glfwDestroyWindow(m_Window);
		--s_GLFWWindowCount;

		if (s_GLFWWindowCount == 0)
			Window::ShutdownAPI();
	}

	void Win_Window::Update() {
		NB_PROFILE_FUNCTION();
		
		glfwPollEvents();
		m_Context->SwapBuffers();
	}

	void Win_Window::SetVSync(bool enabled) {
		if (Renderer::GetAPI() != RendererAPI::API::Vulkan)
		{
			if (enabled)
				glfwSwapInterval(1);
			else
				glfwSwapInterval(0);

			m_Data.Vsync = enabled;
		}
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

	GLFWmonitor* Win_Window::FindBestMonitor() {
		int monitorNumber, bestoverlap = 0;
		int mx, my;

		GLFWmonitor* bestmonitor = glfwGetPrimaryMonitor();
		GLFWmonitor** monitors = glfwGetMonitors(&monitorNumber);

		glfwGetWindowPos(m_Window, (int*)&m_Data.PosX, (int*)&m_Data.PosY);
		glfwGetWindowSize(m_Window, (int*)&m_Data.Width, (int*)&m_Data.Height);
		
		for (int i = 0; i < monitorNumber; i++) {
			const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
			glfwGetMonitorPos(monitors[i], &mx, &my);
			
			int overlap =
				Maths::Max(0, Maths::Min(int(m_Data.PosX + m_Data.Width ), mx + mode->width ) - Maths::Max(int(m_Data.PosX), mx)) *
				Maths::Max(0, Maths::Min(int(m_Data.PosY + m_Data.Height), my + mode->height) - Maths::Max(int(m_Data.PosY), my));

			if (bestoverlap < overlap) {
				bestoverlap = overlap;
				bestmonitor = monitors[i];
			}
		}

		return bestmonitor;
	}

	void Win_Window::SetFullscreen(bool fullscreen) {
		if (fullscreen && !m_Data.Fullscreen) {
			glfwGetWindowPos(m_Window, (int*)&m_PreviousData.PosX, (int*)&m_PreviousData.PosY);
			glfwGetWindowSize(m_Window, (int*)&m_PreviousData.Width, (int*)&m_PreviousData.Height);

			GLFWmonitor* monitor = FindBestMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);

			glfwSetWindowPos(m_Window, 0, 0);
			glfwSetWindowSize(m_Window, mode->width, mode->height);
			glfwWindowHint(GLFW_DECORATED, false);

			//glfwSetWindowMonitor(m_Window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
		}
		else if (m_Data.Fullscreen) {
			glfwSetWindowMonitor(m_Window, nullptr, m_PreviousData.PosX, m_PreviousData.PosY, m_PreviousData.Width, m_PreviousData.Height, GLFW_DONT_CARE);
		}

		m_Data.Fullscreen = fullscreen;
	}
}