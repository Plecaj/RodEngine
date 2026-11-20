#include "rdpch.h"

#include "Rod/Core/Application.h"
#include "Rod/Project/ProjectExporter.h"
#include "Rod/Project/ProjectSerializer.h"
#include "Rod/Utils/PlatformUtils.h"
#include "RuntimeLayer.h"

#include <optional>
#include <stdexcept>

namespace Rod {

	auto CreateApplication(const std::optional<Project>& project) -> Scope<Application>
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
		const auto activeProject = app->GetProject();
		if (!activeProject)
			throw std::logic_error("Rod Runtime requires a project");
		app->PushLayer(CreateScope<RuntimeLayer>(*activeProject, app->GetWindow(), app->GetInput(), requestClose));
		return app;
	}

}
