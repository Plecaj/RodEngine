#pragma once
#include <filesystem>
#include <string>

namespace Rod {

	struct ProjectProperties
	{
		std::string Name = "Sandbox";
		std::filesystem::path AssetsPath = "Assets";
		std::filesystem::path StartupScenePath = "Scenes/Main.rod";
		std::filesystem::path ScriptsProjectPath = "Scripts/Game.csproj";
	};
}
