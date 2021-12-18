#pragma once

#include "Nebula/Core/Layer.h"

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

		void SetBlockEvents(bool block) { m_BlockEvents = block; }
	private:
		void SetDarkThemeColour();
	private:
		bool m_BlockEvents;
	};
}