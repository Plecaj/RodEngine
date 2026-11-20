#include "rdpch.h"

#include "Rod/Core/Log.h"
#include "Rod/Project/Project.h"
#include "Rod/Project/ProjectExporter.h"
#include "Rod/Project/ProjectSerializer.h"
#include "Rod/Scripting/ScriptBuilder.h"
#include "Rod/Utils/PlatformUtils.h"

#include <cstdio>
#include <exception>
#include <stdexcept>

auto main(int argc, char* argv[]) -> int
{
	constexpr int argumentCount = 3;
	if (argc != argumentCount)
	{
		std::fputs("Usage: Rod-Package <project.rodproj> <output-directory>\n", stderr);
		return 1;
	}
	try
	{
		Rod::Log::Init();
		Rod::Project project{std::filesystem::absolute(argv[1]).lexically_normal(), {}};
		Rod::ProjectSerializer::Deserialize(project);
		const auto scripts = Rod::ScriptBuilder::BuildProject(project.GetScriptProjectPath());
		if (!scripts.Success)
			throw std::runtime_error("Script build failed: " + scripts.Output);
		const auto directory = Rod::Platform::GetExecutablePath().parent_path();
		constexpr int outputArgument = 2;
		Rod::ProjectExporter::Export(project, directory / "Rod-Runtime" ROD_EXECUTABLE_SUFFIX, argv[outputArgument]);
		return 0;
	}
	catch (const std::exception& exception)
	{
		std::fprintf(stderr, "Game export failed: %s\n", exception.what());
		return 1;
	}
}
