#pragma once

#include "Rod/Core/Timestep.h"
#include "Rod/Events/Event.h"

#include <string>

namespace Rod {

	class Layer
	{
	public:
		explicit Layer(std::string name = "Layer") noexcept;
		virtual ~Layer();
		Layer(const Layer&) = delete;
		auto operator=(const Layer&) -> Layer& = delete;
		Layer(Layer&&) = delete;
		auto operator=(Layer&&) -> Layer& = delete;

		virtual void OnUpdate(Timestep){};
		virtual void OnImGuiRender() {};
		virtual void OnEvent(Event&) {};

		[[nodiscard]] auto GetName() const noexcept -> const std::string& { return m_DebugName; }

	private:
		std::string m_DebugName;
	};

}
