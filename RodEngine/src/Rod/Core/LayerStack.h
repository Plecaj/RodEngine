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
		auto operator=(const LayerStack&) -> LayerStack& = delete;
		LayerStack(LayerStack&&) = delete;
		auto operator=(LayerStack&&) -> LayerStack& = delete;

		void PushLayer(Scope<Layer> layer);
		void PushOverlay(Scope<Layer> overlay);
		void PopLayer(Layer& layer);
		void PopOverlay(Layer& overlay);
		void Clear() noexcept;

		[[nodiscard]] auto begin() const noexcept -> std::vector<Scope<Layer>>::const_iterator
		{
			return m_Layers.begin();
		}
		[[nodiscard]] auto end() const noexcept -> std::vector<Scope<Layer>>::const_iterator { return m_Layers.end(); }

	private:
		std::vector<Scope<Layer>> m_Layers;
		std::vector<Scope<Layer>>::difference_type m_LayerInsertIndex = 0;
	};

}
