#include "RuntimeLayer.h"

#include "Rod/Renderer/RenderCommand.h"

namespace Rod {

	std::filesystem::path RuntimeLayer::ResolveStartupScenePath() const
	{
		std::filesystem::path startupScene = "assets/scenes/Startup.rod";
		if (std::filesystem::exists(startupScene))
			return startupScene;

		return "assets/scenes/Example3D.rod";
	}

	void RuntimeLayer::OnAttach()
	{
		ScriptEngine::Init();

		m_Scene = CreateRef<Scene>();
		std::filesystem::path scenePath = ResolveStartupScenePath();
		if (!std::filesystem::exists(scenePath))
		{
			RD_CORE_ERROR("Runtime startup scene was not found: {}", scenePath.string());
			Application::Get().Close();
			return;
		}

		SceneSerializer serializer(m_Scene);
		if (!serializer.DeserializeText(scenePath.string()))
		{
			RD_CORE_ERROR("Failed to load runtime scene: {}", scenePath.string());
			Application::Get().Close();
			return;
		}

		auto& window = Application::Get().GetWindow();
		m_Scene->OnViewportResize(window.GetWidth(), window.GetHeight());
		m_Scene->OnRuntimeStart();
		m_RunningScene = true;
	}

	void RuntimeLayer::OnDetach()
	{
		if (m_RunningScene && m_Scene)
			m_Scene->OnRuntimeStop();

		ScriptEngine::Shutdown();
	}

	void RuntimeLayer::OnUpdate(Timestep ts)
	{
		ScriptEngine::OnUpdate();

		RenderCommand::SetClearColor({ 0.15f, 0.15f, 0.15f, 1.0f });
		RenderCommand::Clear();
		Renderer2D::ResetStats();

		if (m_RunningScene && m_Scene)
			m_Scene->OnUpdateRuntime(ts);
	}

	void RuntimeLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>(RD_BIND_EVENT_FN(RuntimeLayer::OnWindowResize));
		dispatcher.Dispatch<KeyPressedEvent>(RD_BIND_EVENT_FN(RuntimeLayer::OnKeyPressed));
	}

	bool RuntimeLayer::OnWindowResize(WindowResizeEvent& event)
	{
		if (m_Scene)
			m_Scene->OnViewportResize(event.GetWidth(), event.GetHeight());

		return false;
	}

	bool RuntimeLayer::OnKeyPressed(KeyPressedEvent& event)
	{
		if (event.GetKeyCode() == Key::Escape)
		{
			Application::Get().Close();
			return true;
		}

		return false;
	}

}

