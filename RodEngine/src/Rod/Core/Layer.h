#pragma once

#include "Rod/Core/Timestep.h"
#include "Rod/Events/Event.h"

#include <string>

namespace Rod {

	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {};
		virtual void OnDetach() {};
		virtual void OnUpdate(Timestep) {};
		virtual void OnImGuiRender() {};
		virtual void OnEvent(Event&) {};

		inline const std::string& GetName() const { return m_DebugName; }
	private:
		std::string m_DebugName;
	};


}

