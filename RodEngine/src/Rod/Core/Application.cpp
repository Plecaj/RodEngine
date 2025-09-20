#include "rdpch.h"

#include "Rod/Renderer/Renderer.h"
#include "Application.h"
#include "Log.h"
#include "Input.h"

#include <GLFW/glfw3.h>


namespace Rod {
		
	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string& name, const std::string& iconFilepath, bool _isEditor)
	{
		RD_PROFILE_FUNCTION();
		RD_CORE_ASSERT(!s_Instance, "Application already exists!")
			s_Instance = this;
		
		WindowProps props;
		props.Title = name;
		props._IsEditor = _isEditor;
		props.TaskbarIconFilepath = iconFilepath;
		m_Window = Scope<Window>(Window::Create(props));
		m_Window->SetEventCallback(RD_BIND_EVENT_FN(Application::OnEvent));
		m_Window->SetVSync(false);

		Renderer::Init();

		m_ImGuiLayer = new ImGuiLayer;
		PushOverlay(m_ImGuiLayer);

	}

	Application::~Application()
	{
		RD_PROFILE_FUNCTION();

		Renderer::Shutdown();
	}

	void Application::PushLayer(Layer* layer)
	{
		RD_PROFILE_FUNCTION();

		RD_CORE_INFO("Pushing layer: {0}", layer->GetName());
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		RD_PROFILE_FUNCTION();

		RD_CORE_INFO("Pushing overlay: {0}", overlay->GetName());
		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::Minimalize()
	{
		m_Minimized = true;
		m_Window->Minimalize();
	}

	void Application::Maximalize()
	{
		m_Maximalized = true;
		m_Window->Maximalize();
	}

	void Application::RestoreWindow()
	{
		if(m_Maximalized)
			m_Maximalized = false;

		m_Window->Restore();
	}

	void Application::BeginWindowDrag()
	{
		m_Window->BeginWindowDrag();
	}

	void Application::OnEvent(Event& e)
	{
		RD_PROFILE_FUNCTION();

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
		RD_PROFILE_FUNCTION();
		
		while (m_Running)
		{
			RD_PROFILE_SCOPE("Run loop");

			float time = (float)glfwGetTime();  //TODO: Platform::GetTime()
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!m_Minimized) {
				{
					RD_PROFILE_SCOPE("Layers OnUpdate");

					for (Layer* layer : m_LayerStack)
					{
						layer->OnUpdate(timestep);
					}
				}
			}
			m_ImGuiLayer->Begin();
			{
				RD_PROFILE_SCOPE("ImGui OnUpdate");

				for (Layer* layer : m_LayerStack)
				{
					layer->OnImGuiRender();
				}
				m_ImGuiLayer->End();
			}

			m_Window->OnUpdate();
		};
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		Close();
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		RD_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0) {
			m_Minimized = true;
			return false;
		}
		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}

}