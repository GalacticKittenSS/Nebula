#pragma once

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
		static void RecreateSwapChain();

		static void** GetGraphicsQueue();
		static void** GetPresentQueue();
		static void* GetRenderPass();
	private:
		static void CreateSwapChain();
		static void CreateImageViews();
		static void CreateRenderPass();
		static void CreateFramebuffers();
		static void CleanUpSwapChain();
	private:
		static Settings m_Settings;
	};
}