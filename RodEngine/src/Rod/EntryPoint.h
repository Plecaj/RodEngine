#pragma once

#ifdef RD_PLATFORM_WINDOWS

extern Rod::Application* Rod::CreateApplication();

int main(int argc, char** argv)
{
	auto app = Rod::CreateApplication();
	app->Run();
	delete app;
}

#endif