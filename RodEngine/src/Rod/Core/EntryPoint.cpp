#include "rdpch.h"

#include "Rod/Core/EntryPoint.h"

#include "Rod/Core/Application.h"
#include "Rod/Core/Log.h"
#include "Rod/Debug/Instrumentator.h"

#include <cstdio>
#include <exception>

auto main(int argc, char* argv[]) -> int
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
