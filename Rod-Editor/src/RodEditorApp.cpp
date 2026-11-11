#include "rdpch.h"

#include "EditorLayer.h"
#include "Rod/Core/EntryPoint.h" // IWYU pragma: keep

namespace Rod {

	class RodEditor : public Application
	{
	public:
		explicit RodEditor(Project project)
		    : Application(
		          WindowProps{
		              "Rod Editor",
		              WindowProps::DEFAULT_WIDTH,
		              WindowProps::DEFAULT_HEIGHT,
		              "assets/textures/logo.png",
		              true},
		          std::move(project))
		{
			PushLayer(new EditorLayer(*GetProject()));
		}

		[[nodiscard]] virtual bool IsEditor() const override { return true; }
	};

	Application* CreateApplication(const std::optional<Project>& project)
	{
		RD_CORE_ASSERT(project.has_value(), "Project must be provided for editor");
		return new RodEditor(project.value());
	}

}
