#include "rdpch.h"

#include "EditorLayer.h"
#include "Rod/Core/EntryPoint.h" // IWYU pragma: keep

namespace Rod {

	class RodEditor : public Application
	{
	public:
		RodEditor() : Application("Rod Editor", "assets/textures/logo.png", true) { PushLayer(new EditorLayer()); }

		[[nodiscard]] virtual bool IsEditor() const override { return true; }
	};

	Application* CreateApplication()
	{
		return new RodEditor();
	}

}
