#pragma once

#include "API.h"
#include "Nebula/Events/Event.h"
#include "Nebula/Utils/Time.h"

namespace Nebula {
	class NB_API Layer {
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void Attach() { }
		virtual void Detach() { }

		virtual void OnEvent(Event& e) { }

		virtual void Update(Timestep ts) { }
		virtual void Render() { }
		virtual void ImGuiRender() { }

		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
}