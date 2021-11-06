#pragma once

#include "Nebula/Window.h"

#include "Nebula/events/Key_Event.h"
#include "Nebula/events/Mouse_Event.h"
#include "Nebula/events/Window_Event.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

//Window Generator for Windows Systems using glfw
namespace Nebula {
	class Win_Window : public Window {
	public:
		Win_Window(const WindowProps& props);
		virtual ~Win_Window();

		void Update() override;

		inline uint32_t GetWidth() const override { return m_Data.Width; }
		inline uint32_t GetHeight() const override { return m_Data.Height; }

		//Window Attributes
		inline void SetEventCallback(const EventCallbackFn& callback) override {
			m_Data.EventCallback = callback;
		}

		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		inline virtual void* GetNativeWindow() const override { return m_Window; }
	private:
		virtual void Init(const WindowProps& props);
		virtual void ShutDown();
	private:
		GLFWwindow* m_Window;

		struct WindowData {
			std::string Title;
			uint32_t Width, Height;
			bool Vsync;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};
}