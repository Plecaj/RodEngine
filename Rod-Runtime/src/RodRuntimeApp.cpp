#include <Rod.h>
#include "Rod/Core/EntryPoint.h"

#include "RuntimeLayer.h"

namespace Rod {

	class RodRuntime : public Application
	{
	public:
		RodRuntime()
			: Application("Rod Game")
		{
			PushLayer(new RuntimeLayer());
		}
	};

	Application* CreateApplication()
	{
		return new RodRuntime();
	}

}

