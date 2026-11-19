#pragma once

#include <filesystem>
#include <optional>

namespace Rod {

	class Project;
	class Scene;

	class ProjectExporter
	{
	public:
		static constexpr auto s_ManifestFilename = "Game.rodproj";

		static void Export(
		    const Project& project,
		    const std::filesystem::path& runtimeExecutable,
		    const std::filesystem::path& outputDirectory,
		    std::optional<Scene&> startupScene = std::nullopt);
	};

}
