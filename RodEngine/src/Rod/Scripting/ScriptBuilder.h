#pragma once

#include <filesystem>
#include <string>

namespace Rod {

	struct ScriptBuildResult
	{
		bool Success = false;
		std::string Output;
	};

	class ScriptBuilder
	{
	public:
		[[nodiscard]] static std::filesystem::path GetAssemblyPath(const std::filesystem::path& projectPath);
		[[nodiscard]] static ScriptBuildResult BuildProject(const std::filesystem::path& projectPath);
	};
}
