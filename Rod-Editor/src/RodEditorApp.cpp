#include "rdpch.h"

#include "EditorLayer.h"
#include "Rod/Core/Application.h"

#include <stdexcept>

namespace Rod {

	auto CreateApplication(const std::optional<Project>& project) -> Scope<Application>
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
		const auto editorProject = app->GetProject();
		const auto imGuiLayer = app->GetImGuiLayer();
		if (!editorProject || !imGuiLayer)
			throw std::logic_error("Rod Editor requires a project and an ImGui layer");
		app->PushLayer(
		    CreateScope<EditorLayer>(*editorProject, app->GetWindow(), app->GetInput(), *imGuiLayer, requestClose));
		return app;
	}

}
