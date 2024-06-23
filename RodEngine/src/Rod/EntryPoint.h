#pragma once

#ifdef RD_PLATFORM_WINDOWS

extern Rod::Application* Rod::CreateApplication();

int main(int argc, char** argv)
{
	Rod::Log::Init();
	RD_CORE_WARN ("Initialized Log!");


	auto app = Rod::CreateApplication();
	app->Run();
	delete app;

}

#endif