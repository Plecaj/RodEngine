#include "rdpch.h"

#include "ScriptBuilder.h"

#include "Rod/Utils/Process.h"

namespace Rod {

	auto ScriptBuilder::GetAssemblyPath(const std::filesystem::path& projectPath) -> std::filesystem::path
	{
		return projectPath.parent_path() / "bin/Debug/net9.0" / (projectPath.stem().string() + ".dll");
	}

	auto ScriptBuilder::BuildProject(const std::filesystem::path& projectPath) -> ScriptBuildResult
	{
		if (!std::filesystem::exists(projectPath))
			return {.Success = false, .Output = "Script project does not exist: " + projectPath.string()};

		auto result = Process::Run({"dotnet", "build", projectPath.string(), "--nologo"});
		return {.Success = result.ExitCode == 0, .Output = std::move(result.Output)};
	}

}
