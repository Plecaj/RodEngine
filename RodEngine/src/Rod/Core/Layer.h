#pragma once

#include "Rod/Core/Timestep.h"
#include "Rod/Events/Event.h"

#include <string>

namespace Rod {

	class Layer
	{
	public:
		explicit Layer(std::string name = "Layer");
		virtual ~Layer();
		Layer(const Layer&) = delete;
		Layer& operator=(const Layer&) = delete;
		Layer(Layer&&) = delete;
		Layer& operator=(Layer&&) = delete;

		virtual void OnAttach() {};
		virtual void OnDetach() {};
		virtual void OnUpdate(Timestep){};
		virtual void OnImGuiRender() {};
		virtual void OnEvent(Event&) {};

		[[nodiscard]] const std::string& GetName() const noexcept { return m_DebugName; }

	private:
		std::string m_DebugName;
	};

}
