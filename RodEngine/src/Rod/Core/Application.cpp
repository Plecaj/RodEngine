#include "rdpch.h"

#include "Application.h"

#include "Rod/Core/Layer.h"
#include "Rod/Core/Timestep.h"
#include "Rod/Core/Window.h"
#include "Rod/Events/ApplicationEvent.h"
#include "Rod/Events/Event.h"
#include "Rod/ImGui/ImGuiLayer.h"
#include "Rod/Renderer/SceneRenderer.h"
#include "Rod/Utils/PlatformUtils.h"

namespace Rod {

	Application* Application::s_Instance = nullptr;

	Application::Application(const WindowProps& props, std::optional<Project> project)
	    : m_Project(std::move(project))
	{
		RD_PROFILE_FUNCTION();
		RD_CORE_ASSERT(!s_Instance, "Application already exists!")
		s_Instance = this;

		m_Window = Window::Create(props);
		m_Window->SetEventCallback(RD_BIND_EVENT_FN(Application::OnEvent));
		m_Window->SetVSync(false);

		SceneRenderer::Init();

		if (props.UsesCustomUi)
		{
			m_ImGuiLayer = new ImGuiLayer;
			PushOverlay(m_ImGuiLayer);
		}
	}

	Application::~Application()
	{
		RD_PROFILE_FUNCTION();

		SceneRenderer::Shutdown();
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

	void Application::Minimize()
	{
		m_Minimized = true;
		m_Window->Minimize();
	}

	void Application::Maximize()
	{
		m_Maximized = true;
		m_Window->Maximize();
	}

	void Application::RestoreWindow()
	{
		m_Maximized = false;

		m_Window->Restore();
	}

	void Application::BeginWindowDrag()
	{
		if (m_Maximized)
			RestoreWindow();

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

			const Timestep timestep = CalculateTimestep();
			UpdateLayers(timestep);
			RenderImGui();
			m_Window->OnUpdate();
		}
	}

	Timestep Application::CalculateTimestep()
	{
		const float time = Platform::GetTime();
		Timestep timestep{time - m_LastFrameTime};
		m_LastFrameTime = time;

		return timestep;
	}

	void Application::UpdateLayers(Timestep timestep)
	{
		if (m_Minimized)
			return;

		RD_PROFILE_SCOPE("Layers OnUpdate");

		for (Layer* layer : m_LayerStack)
			layer->OnUpdate(timestep);
	}

	void Application::RenderImGui()
	{
		if (!m_ImGuiLayer)
			return;

		m_ImGuiLayer->Begin();

		{
			RD_PROFILE_SCOPE("ImGui OnUpdate");

			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
		}

		m_ImGuiLayer->End();
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		(void)e;

		Close();
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		RD_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;
		SceneRenderer::OnViewportResize(e.GetWidth(), e.GetHeight());

		return false;
	}

}
