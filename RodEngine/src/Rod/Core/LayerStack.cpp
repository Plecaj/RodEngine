#include "rdpch.h"

#include "LayerStack.h"

namespace Rod {

	LayerStack::LayerStack() noexcept = default;

	LayerStack::~LayerStack()
	{
		Clear();
	}

	void LayerStack::PushLayer(Scope<Layer> layer)
	{
		RD_CORE_ASSERT(layer, "Cannot push a null layer");
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, std::move(layer));
		m_LayerInsertIndex++;
	}

	void LayerStack::PushOverlay(Scope<Layer> overlay)
	{
		RD_CORE_ASSERT(overlay, "Cannot push a null overlay");
		m_Layers.emplace_back(std::move(overlay));
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		auto layerEnd = m_Layers.begin() + m_LayerInsertIndex;
		auto it = std::find_if(m_Layers.begin(), layerEnd, [layer](const auto& entry) { return entry.get() == layer; });
		if (it != layerEnd)
		{
			m_Layers.erase(it);
			m_LayerInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(Layer* overlay)
	{
		auto overlayBegin = m_Layers.begin() + m_LayerInsertIndex;
		auto it =
		    std::find_if(overlayBegin, m_Layers.end(), [overlay](const auto& entry) { return entry.get() == overlay; });
		if (it != m_Layers.end())
		{
			m_Layers.erase(it);
		}
	}

	void LayerStack::Clear() noexcept
	{
		for (auto& layer : m_Layers)
			layer.reset();
		m_Layers.clear();
		m_LayerInsertIndex = 0;
	}

}
