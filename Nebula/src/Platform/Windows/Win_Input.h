#pragma once

#include "Nebula/Core/Input.h"

namespace Nebula {
	class NB_API Win_Input : public Input {
	public:
	protected:
		virtual bool IsKeyPressedImpl(KeyCode keycode) override;
		virtual bool IsMouseButtonPressedImpl(MouseCode button) override;

		virtual std::pair<float, float> GetMousePosImpl() override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;
	};
}