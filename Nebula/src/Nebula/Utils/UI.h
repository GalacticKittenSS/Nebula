#pragma once

#include "imgui.h"

namespace Nebula::UI {

	struct ScopedStyleColor
	{
		ScopedStyleColor() = default;

		ScopedStyleColor(ImGuiCol idx, ImVec4 colour, bool predicate = true)
			: m_Set(predicate)
		{
			if (predicate)
				ImGui::PushStyleColor(idx, colour);
		}
		
		ScopedStyleColor(ImGuiCol idx, ImU32 colour, bool predicate = true)
			: m_Set(predicate)
		{
			if (predicate)
				ImGui::PushStyleColor(idx, colour);
		}

		~ScopedStyleColor()
		{
			if (m_Set)
				ImGui::PopStyleColor();
		}
	private:
		bool m_Set = false;
	};

	struct ScopedStyleVar
	{
		ScopedStyleVar() = default;

		ScopedStyleVar(ImGuiStyleVar idx, ImVec2 val, bool predicate = true)
			: m_Set(predicate)
		{
			if (predicate)
				ImGui::PushStyleVar(idx, val);
		}

		ScopedStyleVar(ImGuiStyleVar idx, float val, bool predicate = true)
			: m_Set(predicate)
		{
			if (predicate)
				ImGui::PushStyleVar(idx, val);
		}

		~ScopedStyleVar()
		{
			if (m_Set)
				ImGui::PopStyleVar();
		}
	private:
		bool m_Set = false;
	};
}