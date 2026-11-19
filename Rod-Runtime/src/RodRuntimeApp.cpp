#include "rdpch.h"

#include "Rod/Core/Application.h"
#include "Rod/Core/EntryPoint.h" // IWYU pragma: keep
#include "Rod/Project/ProjectExporter.h"
#include "Rod/Project/ProjectSerializer.h"
#include "Rod/Utils/PlatformUtils.h"
#include "RuntimeLayer.h"

#include <optional>

namespace Rod {

	Scope<Application> CreateApplication(const std::optional<Project>& project)
	{
		const auto directory = Platform::GetExecutablePath().parent_path();
		Project runtimeProject = project ? *project : Project{directory / ProjectExporter::s_ManifestFilename, {}};
		if (!project)
			ProjectSerializer::Deserialize(runtimeProject);
		std::filesystem::current_path(directory);

		auto app = CreateScope<Application>(WindowProps{"Rod Game"}, std::move(runtimeProject));
		const auto requestClose = [application = app.get()] {
			application->Close();
		};
		app->PushLayer(CreateScope<RuntimeLayer>(*app->GetProject(), app->GetWindow(), app->GetInput(), requestClose));
		return app;
	}

}
