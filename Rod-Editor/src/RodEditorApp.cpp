#include <Rod.h>
#include "Rod/Core/EntryPoint.h"

#include <imgui.h>

#include "EditorLayer.h"

namespace Rod {

	class RodEditor : public Application
	{
	public:
		RodEditor()
			: Application("Rod Editor", true)
		{
			PushLayer(new EditorLayer());
		}

		~RodEditor()
		{

		}

	};

	Application* CreateApplication()
	{
		return new RodEditor();
	}

}