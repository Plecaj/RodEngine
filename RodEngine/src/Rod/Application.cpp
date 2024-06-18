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
		while (true);
		
		WindowResizeEvent e(1280, 720);
		// Crashes app, logging events isnt provided yet
		RD_TRACE(e);
	}
}