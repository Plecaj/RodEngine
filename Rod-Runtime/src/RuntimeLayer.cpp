#include "rdpch.h"

#include "RuntimeLayer.h"

#include "Rod/Core/KeyCodes.h"
#include "Rod/Core/Window.h"
#include "Rod/Project/Project.h"
#include "Rod/Renderer/RenderCommand.h"
#include "Rod/Renderer/Renderer2D.h"
#include "Rod/Scene/Scene.h"
#include "Rod/Scene/SceneSerializer.h"
#include "Rod/Scripting/ScriptEngine.h"

#include <exception>
#include <utility>

namespace Rod {

	RuntimeLayer::RuntimeLayer(
	    const Project& project,
	    const Window& window,
	    const Input& input,
	    std::function<void()> requestClose)
	    : m_RequestClose(std::move(requestClose))
	{
		try
		{
			ScriptEngine::Init(project, input, ScriptEngine::Mode::Runtime);
			LoadScene(project, window);
		}
		catch (...)
		{
			ScriptEngine::Shutdown();
			throw;
		}
	}

	void RuntimeLayer::LoadScene(const Project& project, const Window& window)
	{
		m_Scene = CreateScope<Scene>();
		const std::filesystem::path scenePath = project.GetStartupScenePath();
		if (!std::filesystem::exists(scenePath))
		{
			RD_CORE_ERROR("Runtime startup scene was not found: {}", scenePath.string());
			m_RequestClose();
			return;
		}

		const SceneSerializer serializer(*m_Scene, project.GetDirectory());
		try
		{
			serializer.DeserializeText(scenePath.string());
		}
		catch (const std::exception& exception)
		{
			RD_CORE_ERROR("{}", exception.what());
			m_RequestClose();
			return;
		}

		m_Scene->OnViewportResize(window.GetWidth(), window.GetHeight());
		m_Scene->OnRuntimeStart();
		m_RunningScene = true;
	}

	RuntimeLayer::~RuntimeLayer()
	{
		if (m_RunningScene && m_Scene)
			m_Scene->OnRuntimeStop();

		ScriptEngine::Shutdown();
	}

	void RuntimeLayer::OnUpdate(Timestep ts)
	{
		RenderCommand::SetClearColor(s_ClearColor);
		RenderCommand::Clear();
		Renderer2D::ResetStats();

		if (m_RunningScene && m_Scene)
			m_Scene->OnUpdateRuntime(ts);
	}

	void RuntimeLayer::OnEvent(Event& event)
	{
		HandleEvent<WindowResizeEvent>(event, [this](WindowResizeEvent& resizeEvent) {
			return OnWindowResize(resizeEvent);
		});
		HandleEvent<KeyPressedEvent>(event, [this](KeyPressedEvent& keyEvent) { return OnKeyPressed(keyEvent); });
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
			m_RequestClose();
			return true;
		}

		return false;
	}

}
