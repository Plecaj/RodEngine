#pragma once

#include "Layer.h"
#include "Rod/Core/Core.h"

#include <vector>

namespace Rod {

	class LayerStack
	{
	public:
		LayerStack() noexcept;
		~LayerStack();
		LayerStack(const LayerStack&) = delete;
		LayerStack& operator=(const LayerStack&) = delete;
		LayerStack(LayerStack&&) = delete;
		LayerStack& operator=(LayerStack&&) = delete;

		void PushLayer(Scope<Layer> layer);
		void PushOverlay(Scope<Layer> overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);
		void Clear() noexcept;

		[[nodiscard]] std::vector<Scope<Layer>>::const_iterator begin() const noexcept { return m_Layers.begin(); }
		[[nodiscard]] std::vector<Scope<Layer>>::const_iterator end() const noexcept { return m_Layers.end(); }

	private:
		std::vector<Scope<Layer>> m_Layers;
		std::vector<Scope<Layer>>::difference_type m_LayerInsertIndex = 0;
	};

}
