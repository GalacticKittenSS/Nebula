#include "nbpch.h"
#include "Application.h"

namespace Nebula {
	Application* Application::s_Instance = nullptr;

	Application::Application() {
		NB_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT(Application::OnEvent));

		m_ImGui = new ImGuiLayer();
		PushOverlay(m_ImGui);
	}

	Application::~Application() {

	}

	void Application::run() {
		while (m_Running) {
			for (Layer* layer : m_LayerStack)
				layer->Update();

			m_ImGui->Begin();
			for (Layer* layer : m_LayerStack)
				layer->Render();
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
}