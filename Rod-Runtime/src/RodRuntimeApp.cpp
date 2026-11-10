#include "rdpch.h"

#include "Rod/Core/EntryPoint.h" // IWYU pragma: keep
#include "RuntimeLayer.h"

namespace Rod {

	class RodRuntime : public Application
	{
	public:
		explicit RodRuntime(Project project) : Application(WindowProps{"Rod Game"}, std::move(project))
		{
			PushLayer(new RuntimeLayer());
		}
	};

	Application* CreateApplication()
	{
		return new RodRuntime(Project{
		    {},
		    ProjectProperties{
		        .Name = "Sandbox",
		        .AssetsPath = "assets",
		        .StartupScenePath = "scenes/Startup.rod",
		        .ScriptsProjectPath = "Scripts/RodGame.csproj"}});
	}

}
