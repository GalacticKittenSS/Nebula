#pragma once

#include "Nebula/Window.h"

#include <GLFW/glfw3.h>

//Window Generator for Windows Systems using glfw
namespace Nebula {
	class W_Window : public Window {
	public:
		W_Window(const WindowProps& props);
		virtual ~W_Window();

		void OnUpdate() override;

		inline uint32_t GetWidth() const override { return m_Data.Width; }
		inline uint32_t GetHeight() const override { return m_Data.Width; }

		//Window Attributes
		inline void SetEventCallback(const EventCallbackFn& callback) override {
			m_Data.EventCallback = callback;
		}

		void SetVSync(bool enabled) override;
		bool IsVSync() const override;
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