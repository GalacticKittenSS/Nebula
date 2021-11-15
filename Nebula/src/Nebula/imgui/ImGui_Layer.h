#pragma once

#include "Nebula/Core/Layer.h"

namespace Nebula {
	class NB_API ImGuiLayer : public Layer {
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void Attach() override;
		void Detach() override;

		void Begin();
		void End();
		void Render() override;
	private:
		float m_Time = 0.0f;
	};
}