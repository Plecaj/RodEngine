#include "rdpch.h"

#include "EditorLayer.h"
#include "Rod/Core/EntryPoint.h" // IWYU pragma: keep

#include <stdexcept>

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
		              "RodAssets/Textures/logo.png",
		              true},
		          std::move(project))
		{
			PushLayer(CreateScope<EditorLayer>(*GetProject()));
		}

		[[nodiscard]] virtual bool IsEditor() const noexcept override { return true; }
	};

	Application* CreateApplication(const std::optional<Project>& project)
	{
		if (!project)
			throw std::invalid_argument("Project must be provided for editor");
		return new RodEditor(*project);
	}

}
