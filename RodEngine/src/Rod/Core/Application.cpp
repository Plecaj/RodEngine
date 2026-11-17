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

	ObserverPtr<Application> Application::s_Instance = nullptr;

	Application::Application(const WindowProps& props, std::optional<Project> project) : m_Project(std::move(project))
	{
		RD_PROFILE_FUNCTION();
		RD_CORE_ASSERT(!s_Instance, "Application already exists!")
		s_Instance = this;

		m_Window = Window::Create(props);
		m_Window->SetEventCallback([this](Event& event) { OnEvent(event); });
		m_Window->SetVSync(false);

		SceneRenderer::Init(m_Project ? m_Project->GetShadersPath() : ProjectProperties{}.ShadersPath);

		if (props.UsesCustomUi)
		{
			auto imGuiLayer = CreateScope<ImGuiLayer>();
			m_ImGuiLayer = imGuiLayer.get();
			PushOverlay(std::move(imGuiLayer));
		}
	}

	Application::~Application()
	{
		RD_PROFILE_FUNCTION();

		m_LayerStack.Clear();
		m_ImGuiLayer = nullptr;
		SceneRenderer::Shutdown();
	}

	void Application::PushLayer(Scope<Layer> layer)
	{
		RD_PROFILE_FUNCTION();

		RD_CORE_ASSERT(layer, "Cannot push a null layer");
		RD_CORE_INFO("Pushing layer: {0}", layer->GetName());
		m_LayerStack.PushLayer(std::move(layer));
	}

	void Application::PushOverlay(Scope<Layer> overlay)
	{
		RD_PROFILE_FUNCTION();

		RD_CORE_ASSERT(overlay, "Cannot push a null overlay");
		RD_CORE_INFO("Pushing overlay: {0}", overlay->GetName());
		m_LayerStack.PushOverlay(std::move(overlay));
	}

	void Application::Close() noexcept
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

		HandleEvent<WindowCloseEvent>(e, [this](WindowCloseEvent& event) { return OnWindowClose(event); });
		HandleEvent<WindowResizeEvent>(e, [this](WindowResizeEvent& event) { return OnWindowResize(event); });

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

		for (const auto& layer : m_LayerStack)
			layer->OnUpdate(timestep);
	}

	void Application::RenderImGui()
	{
		if (!m_ImGuiLayer)
			return;

		m_ImGuiLayer->Begin();

		{
			RD_PROFILE_SCOPE("ImGui OnUpdate");

			for (const auto& layer : m_LayerStack)
				layer->OnImGuiRender();
		}

		m_ImGuiLayer->End();
	}

	bool Application::OnWindowClose(WindowCloseEvent& e) noexcept
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
