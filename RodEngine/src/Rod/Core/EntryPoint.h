#pragma once

#include "Rod/Core/Application.h"
#include "Rod/Core/Core.h"
#include "Rod/Core/Log.h"
#include "Rod/Debug/Instrumentator.h"

#include <cstdio>
#include <exception>

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

	try
	{
		Rod::Log::Init();
		RD_CORE_WARN("Initialized Log!");

		RD_PROFILE_BEGIN_SESSION("Startup", "RodProfile-Startup.json");
		Rod::Scope<Rod::Application> app(Rod::CreateApplication());
		RD_PROFILE_END_SESSION();

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
