#pragma once

#include "Window.h"

#include "Layer_Stack.h"
#include "imgui/ImGui_Layer.h"

#include "events/Event.h"
#include "events/Window_Event.h"

namespace Nebula {
	class NB_API Application {
	public:
		Application();
		~Application();

		void run();
		void OnEvent(Event& e);

		virtual void Render() = 0;
		//virtual void Start() = 0;
		//virtual void Update() { }
		//virtual void Tick()   { }

		virtual void RecieveEvent(Event& e) { }

		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopOverlay(Layer* overlay);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);
	private:
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGui;
		bool m_Running = true;
		LayerStack m_LayerStack;

		static Application* s_Instance;
	};

	//Defined In Client
	Application* createApplication();
}