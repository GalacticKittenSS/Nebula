#include "nbpch.h"
#include "Application.h"

#include "Nebula/Renderer/Renderer.h"
#include "Nebula/Scripting/ScriptEngine.h"

namespace Nebula {
	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specification) : m_Specification(specification) {
		NB_PROFILE_FUNCTION();

		NB_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		if (!m_Specification.WorkingDirectory.empty())
			std::filesystem::current_path(m_Specification.WorkingDirectory);

		Time::Init();
		Window::InitAPI();
		Renderer::Init();
		ScriptEngine::Init();

		m_Window = Window::Create(WindowProps(m_Specification.Name));
		m_Window->SetEventCallback(BIND_EVENT(Application::OnEvent));

		SceneRenderer::Setup();

		m_ImGui = new ImGuiLayer();
		PushOverlay(m_ImGui);
	}

	Application::~Application() {
		NB_PROFILE_FUNCTION();

		ScriptEngine::Shutdown();
		SceneRenderer::CleanUp();
		Renderer::Shutdown();
		
		//GLFW is still needed during Application Close
		//Window::ShutdownAPI();
	}

	void Application::run() {
		NB_PROFILE_FUNCTION();

		while (m_Running) 
		{
			NB_PROFILE_SCOPE("Frame - Application::run()");
			
			SceneRenderer::Render();

			/*Time::Update();
			ExecuteMainThreadQueue();

			if (!m_Minimized) {
				for (Layer* layer : m_LayerStack) {
					layer->Update(Time::DeltaTime());
					layer->Render();
				}
			}*/

			/*m_ImGui->Begin();
			for (Layer* layer : m_LayerStack)
				layer->ImGuiRender();
			m_ImGui->End();*/

			m_Window->Update();
		}
	}

	void Application::SubmitToMainThread(const std::function<void()>& function)
	{
		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);
		m_MainThreadQueue.emplace_back(function);
	}

	void Application::ExecuteMainThreadQueue()
	{
		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);
		
		for (auto& func : m_MainThreadQueue)
			func();

		m_MainThreadQueue.clear();
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
		SceneRenderer::OnWindowResize(e);

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
