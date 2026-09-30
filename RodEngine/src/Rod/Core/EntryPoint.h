#pragma once

#include "Rod/Debug/Instrumentator.h"

#ifdef RD_PLATFORM_WINDOWS

extern Rod::Application* Rod::CreateApplication();

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

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
#endif
