#pragma once

#include "Window.h"

#include "Layer_Stack.h"
#include "Nebula/imgui/ImGui_Layer.h"

#include "Nebula/Events/Event.h"
#include "Nebula/Events/Window_Event.h"
#include "Nebula/Events/Key_Event.h"

#include "Time.h"

int main(int argc, char** argv);

namespace Nebula {
	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			NB_ASSERT(index < Count, "");
			return Args[index];
		}
	};

	struct ApplicationSpecification
	{
		std::string Name = "Nebula App";
		ApplicationCommandLineArgs CommandLineArgs;
		std::string WorkingDirectory;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);
		virtual ~Application();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopOverlay(Layer* overlay);

		void Close();

		ImGuiLayer* GetImGuiLayer() { return m_ImGui; }

		inline static Application& Get() { return *s_Instance; }
		ApplicationSpecification GetSpecification() const { return m_Specification; }
		inline Window& GetWindow() { return *m_Window; }
	private:
		void run();
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnKeyPressed(KeyPressedEvent& e);
	private:
		ApplicationSpecification m_Specification;
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGui;
		bool m_Running = true, m_Minimized = false;
		LayerStack m_LayerStack;

		Timestep m_Timestep;
		float m_LastFrameTime;

		static Application* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	//Defined In Client
	Application* CreateApplication(ApplicationCommandLineArgs args);
}
