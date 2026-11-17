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
		constexpr int argumentCount = 2;
		if (argc != argumentCount)
			throw std::invalid_argument("Expected a single .rodproj manifest argument");

		Rod::Project project{std::filesystem::absolute(argv[1]).lexically_normal(), {}};
		Rod::ProjectSerializer::Deserialize(project);
		return project;
	}
}

int main(int argc, char** argv)
{
	try
	{
		Rod::Log::Init();
		RD_CORE_WARN("Initialized Log!");

		RD_PROFILE_BEGIN_SESSION("Startup", "RodProfile-Startup.json");
		auto app = Rod::CreateApplication(Utils::GetProjectDataFromArguments(argc, argv));
		RD_PROFILE_END_SESSION();

		RD_PROFILE_BEGIN_SESSION("Running", "RodProfile-Running.json");
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
