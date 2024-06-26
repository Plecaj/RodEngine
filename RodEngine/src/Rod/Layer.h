#pragma once

#include "Rod/Core.h"
#include "Rod/Events/Event.h"

namespace Rod {

	class ROD_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {};
		virtual void OnDetach() {};
		virtual void OnUpdate() {};
		virtual void OnEvent(Event& event) {};

		inline const std::string& GetName() const { return m_DebugName; }
	private:
		std::string m_DebugName;
	};


}

