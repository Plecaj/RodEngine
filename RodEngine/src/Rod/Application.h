#pragma once

#include "Rod/Core.h"

#include "Rod/Window.h"
#include "Rod/LayerStack.h"
#include "Rod/Events/Event.h"
#include "Rod/Events/ApplicationEvent.h"

#include "Rod/Core/Timestep.h"

#include "Rod/ImGui/ImGuiLayer.h"

namespace Rod {

	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);
		
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline Window& GetWindow() { return *m_Window; };

		static inline Application& Get() { return *s_Instance; };
	private:
		bool OnWindowClose(WindowCloseEvent& e);
	private:
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;
	private:
		static Application* s_Instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}


