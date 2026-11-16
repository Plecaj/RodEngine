#pragma once

#include <filesystem>

namespace Rod {

	class Project;
	class Scene;

	class ProjectExporter
	{
	public:
		static constexpr auto MANIFEST_FILENAME = "Game.rodproj";

		static void Export(
		    const Project& project,
		    const std::filesystem::path& runtimeExecutable,
		    const std::filesystem::path& outputDirectory,
		    Scene* startupScene = nullptr);
	};

}
