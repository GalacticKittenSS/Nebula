#pragma once

#include "Nebula/Core/Window.h"
#include "Nebula/Renderer/Graphics_Context.h"

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

		void LockAspectRatio(uint8_t width, uint8_t height) override;
		void UnlockAspectRatio() override;

		void SetFullscreen(bool fullscreen) override;
		bool IsFullscreen() override { return m_Data.Fullscreen; }

		inline virtual void* GetSurface() const override { return m_Context->GetSurface(); }
		inline virtual void* GetNativeWindow() const override { return m_Window; }
	private:
		void Init(const WindowProps& props);
		void ShutDown();
		static void InitGLFW();
		GLFWmonitor* FindBestMonitor();
	private:
		GLFWwindow* m_Window;
		Scope<GraphicsContext> m_Context;

		struct WindowData {
			std::string Title;
			
			uint32_t Width, Height;
			uint32_t PosX, PosY;

			bool Vsync;
			bool Fullscreen = false;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
		WindowData m_PreviousData;

		friend Window;
	};
}