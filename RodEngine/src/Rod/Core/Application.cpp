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

	Application::Application(const WindowProps& props, std::optional<Project> project)
	    : m_Project(std::move(project)), m_Window(Window::Create(props)), m_Input(*m_Window)
	{
		RD_PROFILE_FUNCTION();

		m_Window->SetEventCallback([this](Event& event) { OnEvent(event); });
		m_Window->SetVSync(false);

		SceneRenderer::Init(m_Project ? m_Project->GetShadersPath() : ProjectProperties{}.ShadersPath);

		if (props.UsesCustomUi)
		{
			auto imGuiLayer = CreateScope<ImGuiLayer>(*m_Window);
			m_ImGuiLayer = *imGuiLayer;
			PushOverlay(std::move(imGuiLayer));
		}
	}

	Application::~Application()
	{
		RD_PROFILE_FUNCTION();

		m_LayerStack.Clear();
		m_ImGuiLayer.reset();
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

	auto Application::CalculateTimestep() -> Timestep
	{
		const float time = Platform::GetTime();
		Timestep timestep{time - m_LastFrameTime};
		m_LastFrameTime = time;

		return timestep;
	}

	void Application::UpdateLayers(Timestep timestep)
	{
		if (m_Window->IsMinimized() || m_Window->GetWidth() == 0 || m_Window->GetHeight() == 0)
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

	auto Application::OnWindowClose(WindowCloseEvent& e) noexcept -> bool
	{
		(void)e;

		Close();
		return true;
	}

	auto Application::OnWindowResize(WindowResizeEvent& e) -> bool
	{
		RD_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
			return false;

		SceneRenderer::OnViewportResize(e.GetWidth(), e.GetHeight());

		return false;
	}

}
