#include "rdpch.h"

#include "EditorLayer.h"
#include "Rod/Core/Application.h"
#include "Rod/Core/EntryPoint.h" // IWYU pragma: keep

#include <stdexcept>

namespace Rod {

	Scope<Application> CreateApplication(const std::optional<Project>& project)
	{
		if (!project)
			throw std::invalid_argument("Project must be provided for editor");

		auto app = CreateScope<Application>(
		    WindowProps{
		        "Rod Editor",
		        WindowProps::s_DefaultWidth,
		        WindowProps::s_DefaultHeight,
		        "RodAssets/Textures/logo.png",
		        true},
		    project);
		const auto requestClose = [application = app.get()] {
			application->Close();
		};
		app->PushLayer(
		    CreateScope<EditorLayer>(
		        *app->GetProject(),
		        app->GetWindow(),
		        app->GetInput(),
		        *app->GetImGuiLayer(),
		        requestClose));
		return app;
	}

}
