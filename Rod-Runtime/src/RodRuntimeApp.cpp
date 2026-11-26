#include "rdpch.h"

#include "Rod/Core/AppMain.h"
#include "Rod/Core/DesktopHost.h"
#include "Rod/Project/ProjectExporter.h"
#include "Rod/Project/ProjectSerializer.h"
#include "Rod/Renderer/RenderDevice.h"
#include "Rod/Renderer/SceneRenderer.h"
#include "Rod/Renderer/ShaderLoader.h"
#include "Rod/Utils/PlatformUtils.h"
#include "RuntimeApp.h"

auto main(int argc, char* argv[]) -> int
{
	return Rod::RunMain([&] {
		const auto dir = Rod::Platform::GetExecutablePath().parent_path();
		auto project = Rod::ReadProject(argc, argv);
		if (!project)
		{
			auto& manifest = project.emplace(dir / Rod::ProjectExporter::s_ManifestFilename, Rod::ProjectProperties{});
			Rod::ProjectSerializer::Deserialize(manifest);
		}
		std::filesystem::current_path(dir);

		Rod::DesktopHost host(Rod::WindowProps{"Rod Game"});
		const Rod::RenderDevice device;
		device.ConfigureDefaults();
		const auto shaderDirectory = project->GetShadersPath();
		const Rod::ShaderLoader shaders(shaderDirectory, shaderDirectory / "Cached");
		Rod::SceneRenderer renderer(device, shaders);
		Rod::RuntimeApp app(*project, host, device, renderer);
		while (const auto frame = host.NextFrame())
		{
			if (app.Update(*frame))
				host.Present();
		}
	});
}
