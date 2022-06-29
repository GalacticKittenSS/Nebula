#include "nbpch.h"
#include "Application.h"

#include "Nebula/Renderer/Renderer.h"

namespace Nebula {
	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string& name, ApplicationCommandLineArgs args) {
		NB_PROFILE_FUNCTION();

		NB_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		Time::Init();

		m_Window = Window::Create(WindowProps(name));
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

		while (m_Running) 
		{
			NB_PROFILE_SCOPE("Frame - Application::run()");
			
			if (!m_Minimized) {
				for (Layer* layer : m_LayerStack) {
					layer->Update(Time::DeltaTime());
					layer->Render();
				}
			}

			m_ImGui->Begin();
			for (Layer* layer : m_LayerStack)
				layer->ImGuiRender();
			m_ImGui->End();

			m_Window->Update();
			Time::Update();
		}
	}

	void Application::Close() {
		m_Running = false;
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
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT(Application::OnKeyPressed));

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

	bool Application::OnKeyPressed(KeyPressedEvent& e) {
		if (e.GetKeyCode() == NB_F11) {
			m_Window->SetFullscreen(!m_Window->IsFullscreen());
			return true;
		}

		return false;
	}
}
