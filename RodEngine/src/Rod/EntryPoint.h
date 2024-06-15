#pragma once

#ifdef RD_PLATFORM_WINDOWS

extern Rod::Application* Rod::CreateApplication();

int main(int argc, char** argv)
{
	Rod::Log::Init();
	RD_CORE_WARN ("Initialized Log!");
	int a = 5;
	RD_INFO ("Hello! Var={0}", a);

	auto app = Rod::CreateApplication();
	app->Run();
	delete app;
}

#endif