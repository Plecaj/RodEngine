#include "rdpch.h"
#include "ScriptBuilder.h"

#ifdef RD_PLATFORM_WINDOWS
	#include <cstdio>
#endif

namespace Rod {

	ScriptBuildResult ScriptBuilder::BuildProject(const std::filesystem::path& projectPath)
	{
		ScriptBuildResult result;

		if (!std::filesystem::exists(projectPath))
		{
			result.Output = "Script project does not exist: " + projectPath.string();
			return result;
		}

#ifdef RD_PLATFORM_WINDOWS
		std::string command = "dotnet build \"" + projectPath.string() + "\" --nologo 2>&1";
		FILE* pipe = _popen(command.c_str(), "r");
		if (!pipe)
		{
			result.Output = "Failed to start dotnet build.";
			return result;
		}

		char buffer[512];
		while (fgets(buffer, sizeof(buffer), pipe))
			result.Output += buffer;

		result.Success = _pclose(pipe) == 0;
#else
		result.Output = "Script building is currently implemented for Windows.";
#endif
		return result;
	}
}
