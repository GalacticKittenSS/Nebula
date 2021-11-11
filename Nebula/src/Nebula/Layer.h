#pragma once

#include "API.h"
#include "events/Event.h"
#include "core/Timestep.h"

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

		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
}