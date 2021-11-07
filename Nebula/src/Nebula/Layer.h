#pragma once

#include "Nebula/API.h"
#include "Nebula/events/Event.h"

namespace Nebula {
	class NB_API Layer {
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void Attach() { }
		virtual void Detach() { }

		virtual void OnEvent(Event& e) { }

		virtual void Update() { }
		virtual void Render() { }

		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
}