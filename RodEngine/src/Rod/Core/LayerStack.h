#pragma once

#include "Layer.h"

#include <vector>

namespace Rod {

	class LayerStack
	{
	public:
		LayerStack();
		~LayerStack();
		LayerStack(const LayerStack&) = delete;
		LayerStack& operator=(const LayerStack&) = delete;
		LayerStack(LayerStack&&) = delete;
		LayerStack& operator=(LayerStack&&) = delete;

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);

		[[nodiscard]] std::vector<Layer*>::iterator begin() noexcept { return m_Layers.begin(); }
		[[nodiscard]] std::vector<Layer*>::iterator end() noexcept { return m_Layers.end(); }

	private:
		std::vector<Layer*> m_Layers;
		std::vector<Layer*>::difference_type m_LayerInsertIndex = 0;
	};

}
