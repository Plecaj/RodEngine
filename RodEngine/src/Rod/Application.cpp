#include "rdpch.h"

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
		if (e.IsInCategory(EventCategoryApplication)) 
		{
			RD_INFO("{}", e.ToString());
		}
		if (e.IsInCategory(EventCategoryInput))
		{
			RD_INFO("{}", e.ToString());
		}

		while (true) {};
	}

}