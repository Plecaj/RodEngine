#include "Application.h"
#include "Rod/Events/ApplicationEvent.h"
#include "Log.h"

namespace Rod {
	
	Application::Application() 
	{
	}

	Application::~Application()
	{
	}

	void Application::Run() 
	{
		WindowResizeEvent e(1280, 720);
		RD_INFO("{}", e.ToString());

		while (true) {};
	}

}