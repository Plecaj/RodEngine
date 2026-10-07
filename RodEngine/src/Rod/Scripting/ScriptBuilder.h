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
		static ScriptBuildResult BuildProject(const std::filesystem::path& projectPath);
	};
}
