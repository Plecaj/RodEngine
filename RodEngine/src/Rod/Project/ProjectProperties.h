#pragma once
#include <filesystem>
#include <string>

namespace Rod {

	struct ProjectProperties
	{
		std::string Name = "Sandbox";
		std::filesystem::path AssetsPath = "Assets";
		std::filesystem::path StartupScenePath = "Assets/Scenes/Main.rod";
		std::filesystem::path ScriptsProjectPath = "Assets/Scripts/Game.csproj";
		// Bundled engine shaders are the default; custom relative paths use the project directory.
		std::filesystem::path ShadersPath = std::filesystem::absolute("RodAssets/Shaders").lexically_normal();
	};
}
