#include "rdpch.h"

#include "Rod/Core/Log.h"
#include "Rod/Project/Project.h"
#include "Rod/Project/ProjectExporter.h"
#include "Rod/Project/ProjectSerializer.h"
#include "Rod/Scripting/ScriptBuilder.h"
#include "Rod/Utils/PlatformUtils.h"

#include <cstdio>
#include <exception>
#include <span>
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
		const std::span arguments(argv, static_cast<size_t>(argc));
		Rod::Project project{std::filesystem::absolute(arguments.at(1)).lexically_normal(), {}};
		Rod::ProjectSerializer::Deserialize(project);
		const auto scripts = Rod::ScriptBuilder::BuildProject(project.GetScriptProjectPath());
		if (!scripts.Success)
			throw std::runtime_error("Script build failed: " + scripts.Output);
		const auto directory = Rod::Platform::GetExecutablePath().parent_path();
		Rod::ProjectExporter::Export(project, directory / "Rod-Runtime" ROD_EXECUTABLE_SUFFIX, arguments.back());
		return 0;
	}
	catch (const std::exception& exception)
	{
		std::fputs("Game export failed: ", stderr);
		std::fputs(exception.what(), stderr);
		std::fputc('\n', stderr);
		return 1;
	}
}
