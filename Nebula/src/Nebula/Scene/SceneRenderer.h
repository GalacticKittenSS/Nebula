#pragma once

#include "Nebula/Events/Window_Event.h"

namespace Nebula
{
	class SceneRenderer
	{
	public:
		struct Settings
		{
			bool enableValidationLayers = true;
		};
	public:
		static void Setup();
		static void Render();
		static void CleanUp();
		
		static void OnEvent(Event& e);
		static bool OnWindowResize(WindowResizeEvent& e);
	private:
		static Settings m_Settings;
	};
}