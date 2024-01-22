#pragma once

#include "Nebula/Core/Layer.h"
#include "Nebula/Renderer/FrameBuffer.h"
#include "Nebula/Renderer/RenderPass.h"
#include "Nebula/Renderer/CommandBuffer.h"

struct ImVec4;

namespace Nebula {
	class NB_API ImGuiLayer : public Layer {
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void Attach() override;
		void Detach() override;
		void OnEvent(Event& e) override;

		void Begin();
		void End();

		void SetDarkThemeColour();
		void SetLightThemeColour();
		void SetColdThemeColour();
		void SetWarmThemeColour();

		void SetBlockEvents(bool block) { m_BlockEvents = block; }

		uint32_t GetActiveWidgetID() const;
	private:
		void SetColours(ImVec4 primary, ImVec4 text, ImVec4 regular, ImVec4 hovered, ImVec4 active);
	private:
		bool m_BlockEvents;
		Ref<FrameBuffer> m_Framebuffer;
		Ref<RenderPass> m_RenderPass;
		Ref<CommandBuffer> m_CommandBuffer;
	};
}