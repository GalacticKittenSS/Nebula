#include "nbpch.h"
#include "Application.h"

#include "Nebula/Renderer/Renderer.h"

#include <GLFW/glfw3.h>

namespace Nebula {
	Application* Application::s_Instance = nullptr;

	Application::Application() {
		
		NB_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = Window::Create();
		m_Window->SetEventCallback(BIND_EVENT(Application::OnEvent));

		Renderer::Init();

		m_ImGui = new ImGuiLayer();
		PushOverlay(m_ImGui);
	}

	Application::~Application() {
		Renderer::Shutdown();
	}

	void Application::run() {
		while (m_Running) {
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
		m_LayerStack.PushLayer(layer);
	}

	void Application::PopLayer(Layer* layer) {
		m_LayerStack.PopLayer(layer);
	}
	
	void Application::PushOverlay(Layer* overlay) {
		m_LayerStack.PushOverlay(overlay);
	}

	void Application::PopOverlay(Layer* overlay) {
		m_LayerStack.PopOverlay(overlay);
	}

	void Application::OnEvent(Event& e) {
		Dispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT(Application::OnWindowResize));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
			(*--it)->OnEvent(e);

			if (e.Handled)
				break;
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e) {
		if (e.GetWidth() == 0 || e.GetHeight() == 0) {
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}
}