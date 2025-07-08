#include "rdpch.h"

#include "Rod/Renderer/Renderer.h"
#include "Application.h"
#include "Log.h"
#include "Input.h"

#include <GLFW/glfw3.h>

namespace Rod {

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		RD_CORE_ASSERT(!s_Instance, "Application already exists!")
			s_Instance = this;
		
		m_Window = Scope<Window>(Window::Create());
		m_Window->SetEventCallback(RD_BIND_EVENT_FN(Application::OnEvent));
		m_Window->SetVSync(false);

		Renderer::Init();

		m_ImGuiLayer = new ImGuiLayer;
		PushOverlay(m_ImGuiLayer);

	}

	Application::~Application()
	{
	}

	void Application::PushLayer(Layer* layer)
	{
		RD_CORE_INFO("Pushing layer: {0}", layer->GetName());
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		RD_CORE_INFO("Pushing overlay: {0}", overlay->GetName());
		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(RD_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(RD_BIND_EVENT_FN(Application::OnWindowResize));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	void Application::Run()
	{
		while (m_Running)
		{
			float time = (float)glfwGetTime();  //TODO: Platform::GetTime()
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!m_Minimized) {
				for (Layer* layer : m_LayerStack)
				{
					layer->OnUpdate(timestep);
				}
			}
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
			{
				layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		};
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0) {
			m_Minimized = true;
			return false;
		}
		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}

}