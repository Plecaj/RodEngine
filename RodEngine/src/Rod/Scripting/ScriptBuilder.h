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
		[[nodiscard]] static auto GetAssemblyPath(const std::filesystem::path& projectPath) -> std::filesystem::path;
		[[nodiscard]] static auto BuildProject(const std::filesystem::path& projectPath) -> ScriptBuildResult;
	};
}
