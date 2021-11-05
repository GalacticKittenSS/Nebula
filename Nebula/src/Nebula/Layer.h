#pragma once

#include "Nebula/API.h"
#include "Nebula/events/Event.h"

namespace Nebula {
	class NB_API Layer {
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() { }
		virtual void OnDetatch() { }
		virtual void OnEvent(Event& e) { }
		
		virtual void Update() { }

		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
}