#pragma once

#include "API.h"
#include "Layer.h"

namespace Nebula {
	class NB_API LayerStack {
	public:
		LayerStack() = default;
		~LayerStack();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);

		Array<Layer*>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
		Array<Layer*>::reverse_iterator rend() { return m_Layers.rend(); }
		Array<Layer*>::iterator begin() { return m_Layers.begin(); }
		Array<Layer*>::iterator end() { return m_Layers.end(); }

		Array<Layer*>::const_iterator begin() const { return m_Layers.begin(); }
		Array<Layer*>::const_iterator end()	const { return m_Layers.end(); }
		Array<Layer*>::const_reverse_iterator rbegin() const { return m_Layers.rbegin(); }
		Array<Layer*>::const_reverse_iterator rend() const { return m_Layers.rend(); }
	private:
		Array<Layer*> m_Layers;
		uint32_t m_LayerInsertIndex = 0;
	};
}
