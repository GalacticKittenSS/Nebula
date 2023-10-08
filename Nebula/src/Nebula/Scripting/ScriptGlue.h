#pragma once

#include "Nebula/Core/UUID.h"

namespace Nebula {
	struct ScriptFunctionData
	{
		static UUID HoveredEntity;
		
		// ImGui Window Size may be different from Default Window
		static glm::vec2 WindowOffset;
		static glm::vec2 WindowSize;
	};

	class ScriptGlue {
	public:
		static void RegisterFunctions();
		static void RegisterComponents();
	};
}