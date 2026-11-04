#include "rdpch.h"

#include "Layer.h"

namespace Rod {

	Layer::Layer(std::string debugName) : m_DebugName(std::move(debugName))
	{}

	Layer::~Layer() = default;

}
