#include <Rod.h>
#include "Rod/Core/EntryPoint.h"

#include <imgui.h>

#include "EditorLayer.h"

namespace Rod {

	class RodEditor : public Application
	{
	public:
		RodEditor()
			: Application("Rod Editor", "assets/textures/logo.png", true)
		{
			PushLayer(new EditorLayer());
		}

		~RodEditor()
		{

		}

		bool IsEditor() const override { return true; }

	};

	Application* CreateApplication()
	{
		return new RodEditor();
	}

}
