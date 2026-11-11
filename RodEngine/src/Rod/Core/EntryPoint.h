#pragma once

#include "Rod/Core/Application.h"
#include "Rod/Core/Core.h"
#include "Rod/Core/Log.h"
#include "Rod/Debug/Instrumentator.h"
#include "Rod/Project/Project.h"
#include "Rod/Project/ProjectProperties.h"
#include "Rod/Project/ProjectSerializer.h"

#include <cstdio>
#include <exception>
#include <optional>
#include <stdexcept>

namespace Utils {
	[[nodiscard]] inline std::optional<Rod::Project> GetProjectDataFromArguments(int argc, char** argv)
	{
		if (argc <= 1)
			return std::nullopt;
		if (argc == 2)
		{
			Rod::Project project{std::filesystem::absolute(argv[1]).lexically_normal(), {}};
			Rod::ProjectSerializer::Deserialize(project);
			return project;
		}

		if (argc != Rod::Project::GetRequiredMembersCount() + 1)
			throw std::invalid_argument("Entry point argument count does not match required count");

		constexpr int manifestArgument = 1;
		constexpr int nameArgument = 2;
		constexpr int assetsArgument = 3;
		constexpr int startupSceneArgument = 4;
		constexpr int scriptsProjectArgument = 5;
		return Rod::Project{
		    argv[manifestArgument],
		    Rod::ProjectProperties{
		        .Name = argv[nameArgument],
		        .AssetsPath = argv[assetsArgument],
		        .StartupScenePath = argv[startupSceneArgument],
		        .ScriptsProjectPath = argv[scriptsProjectArgument]}};
	}
}

int main(int argc, char** argv)
{
	try
	{
		Rod::Log::Init();
		RD_CORE_WARN("Initialized Log!");

		RD_PROFILE_BEGIN_SESSION("Startup", "RodProfile-Startup.json");
		Rod::Scope<Rod::Application> app(Rod::CreateApplication(Utils::GetProjectDataFromArguments(argc, argv)));
		RD_PROFILE_END_SESSION();

		RD_PROFILE_BEGIN_SESSION("Running", "RodProfile-Startup.json");
		app->Run();
		RD_PROFILE_END_SESSION();

		RD_PROFILE_BEGIN_SESSION("Shutdown", "RodProfile-Shutdown.json");
		app.reset();
		RD_PROFILE_END_SESSION();

		return 0;
	}
	catch (const std::exception& exception)
	{
		std::fprintf(stderr, "RodEngine failed: %s\n", exception.what());
	}
	catch (...)
	{
		std::fputs("RodEngine failed: unknown exception\n", stderr);
	}
	return 1;
}
