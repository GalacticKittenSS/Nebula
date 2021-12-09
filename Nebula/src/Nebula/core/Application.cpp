#include "nbpch.h"
#include "Application.h"

#include "Nebula/Renderer/Renderer.h"

#include <GLFW/glfw3.h>

namespace Nebula {
	Application* Application::s_Instance = nullptr;

	Application::Application() {
		NB_PROFILE_FUNCTION();

		NB_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = Window::Create();
		m_Window->SetEventCallback(BIND_EVENT(Application::OnEvent));

		Renderer::Init();

		m_ImGui = new ImGuiLayer();
		PushOverlay(m_ImGui);
	}

	Application::~Application() {
		NB_PROFILE_FUNCTION();

		Renderer::Shutdown();
	}

	void Application::run() {
		NB_PROFILE_FUNCTION();

		while (m_Running) {
			NB_PROFILE_SCOPE("Frame - Application::run()")

			float time = (float)glfwGetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;
			
			if (!m_Minimized) {
				for (Layer* layer : m_LayerStack) {
					layer->Update(timestep);
					layer->Render();
				}
			}

			m_ImGui->Begin();
			for (Layer* layer : m_LayerStack)
				layer->ImGuiRender();
			m_ImGui->End();

			m_Window->Update();
		}
	}

	void Application::PushLayer(Layer* layer) {
		NB_PROFILE_FUNCTION();

		m_LayerStack.PushLayer(layer);
	}

	void Application::PopLayer(Layer* layer) {
		NB_PROFILE_FUNCTION();

		m_LayerStack.PopLayer(layer);
	}
	
	void Application::PushOverlay(Layer* overlay) {
		NB_PROFILE_FUNCTION();

		m_LayerStack.PushOverlay(overlay);
	}

	void Application::PopOverlay(Layer* overlay) {
		NB_PROFILE_FUNCTION();

		m_LayerStack.PopOverlay(overlay);
	}

	void Application::OnEvent(Event& e) {
		NB_PROFILE_FUNCTION();

		Dispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT(Application::OnWindowResize));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
			if (e.Handled)
				break;
		
			(*it)->OnEvent(e);
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e) {
		NB_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0) {
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}
}