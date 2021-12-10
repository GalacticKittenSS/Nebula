#pragma once

#include "Nebula/Core/API.h"
#include "Nebula/Core/Key_Codes.h"
#include "Nebula/Core/Mouse_Codes.h"

namespace Nebula {
	class NB_API Input {
	public:
		static bool IsKeyPressed(KeyCode key);
		static bool IsMouseButtonPressed(MouseCode button);
		static std::pair<float, float> GetMousePos();
		static float GetMouseX();
		static float GetMouseY();
	};
}