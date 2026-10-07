#include "rdpch.h"
#include "ScriptBuilder.h"

#include "Rod/Utils/Process.h"

namespace Rod {

	ScriptBuildResult ScriptBuilder::BuildProject(const std::filesystem::path& projectPath)
	{
		if (!std::filesystem::exists(projectPath))
			return { false, "Script project does not exist: " + projectPath.string() };

		auto result = Process::Run({ "dotnet", "build", projectPath.string(), "--nologo" });
		return { result.ExitCode == 0, std::move(result.Output) };
	}

}
