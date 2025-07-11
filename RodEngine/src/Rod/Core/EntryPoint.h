#pragma once

#include "Rod/Debug/Instrumentator.h"

#ifdef RD_PLATFORM_WINDOWS

extern Rod::Application* Rod::CreateApplication();

int main(int argc, char** argv)
{
	Rod::Log::Init();
	RD_CORE_WARN("Initialized Log!");

	RD_PROFILE_BEGIN_SESSION("Startup", "RodProfile-Startup.json");
	auto app = Rod::CreateApplication();
	RD_PROFILE_END_SESSION();

	app->Run();

	RD_PROFILE_END_SESSION();
	RD_PROFILE_BEGIN_SESSION("Shutdown", "RodProfile-Shutdown.json");
	delete app;
	RD_PROFILE_END_SESSION();

}
#endif