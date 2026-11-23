#include "rdpch.h"

#include "EditorApp.h"
#include "Rod/Core/AppMain.h"
#include "Rod/Core/DesktopHost.h"
#include "Rod/ImGui/ImGuiSession.h"
#include "Rod/Renderer/SceneRenderer.h"

#include <stdexcept>

auto main(int argc, char* argv[]) -> int
{
	return Rod::RunMain([&] {
		const auto project = Rod::ReadProject(argc, argv);
		if (!project)
			throw std::invalid_argument("Project must be provided for editor");

		Rod::WindowProps props{
		    "Rod Editor",
		    Rod::WindowProps::s_DefaultWidth,
		    Rod::WindowProps::s_DefaultHeight,
		    "RodAssets/Textures/logo.png"};
		props.NativeTitlebar = false;
		Rod::DesktopHost host(props);
		const Rod::SceneRenderer renderer(project->GetShadersPath());
		const Rod::ImGuiSession ui(host.GetWindow());
		Rod::EditorApp editor(*project, host);

		while (const auto frame = host.NextFrame())
		{
			Rod::ImGuiSession::Begin();
			editor.Update(*frame);
			Rod::ImGuiSession::End(frame->Drawable);
			if (frame->Drawable)
				host.Present();
		}
	});
}
