#pragma once

#include "Nebula/Renderer/Shader.h"

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
		
		static void** GetGraphicsQueue();
		static void** GetPresentQueue();
		static Ref<Shader> GetShader();
	private:
		static Settings m_Settings;
	};
}