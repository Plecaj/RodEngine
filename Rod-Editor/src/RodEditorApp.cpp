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
			PushLayer(new EditorLayer());
		}

		[[nodiscard]] virtual bool IsEditor() const override { return true; }
	};

	Application* CreateApplication()
	{
		return new RodEditor(Project{
		    {},
		    ProjectProperties{
		        .Name = "Sandbox",
		        .AssetsPath = "assets",
		        .StartupScenePath = "scenes/Example3D.rod",
		        .ScriptsProjectPath = "Scripts/RodGame.csproj"}});
	}

}
