#include "rdpch.h"

#include "Rod/Core/EntryPoint.h" // IWYU pragma: keep
#include "RuntimeLayer.h"

namespace Rod {

	class RodRuntime : public Application
	{
	public:
		RodRuntime() : Application("Rod Game") { PushLayer(new RuntimeLayer()); }
	};

	Application* CreateApplication()
	{
		return new RodRuntime();
	}

}
