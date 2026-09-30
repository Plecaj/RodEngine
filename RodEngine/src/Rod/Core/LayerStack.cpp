#include "rdpch.h"
#include "LayerStack.h"

namespace Rod {

	LayerStack::LayerStack() = default;
	
	LayerStack::~LayerStack()
	{
		for (Layer* layer : m_Layers)
		{
			layer->OnDetach();
			delete layer;
		}
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
		m_LayerInsertIndex++;
	}

	void LayerStack::PushOverlay(Layer* overlay)
	{
		m_Layers.emplace_back(overlay);
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		auto layerEnd = m_Layers.begin() + m_LayerInsertIndex;
		auto it = std::find(m_Layers.begin(), layerEnd, layer);
		if (it != layerEnd)
		{
			layer->OnDetach();
			m_Layers.erase(it);
			m_LayerInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(Layer* layer)
	{
		auto overlayBegin = m_Layers.begin() + m_LayerInsertIndex;
		auto it = std::find(overlayBegin, m_Layers.end(), layer);
		if (it != m_Layers.end()) 
		{
			layer->OnDetach();
			m_Layers.erase(it);
		}
	}

}
