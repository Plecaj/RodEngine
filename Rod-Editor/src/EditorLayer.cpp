#include "rdpch.h"

#include "EditorLayer.h"

#include "Rod/ImGui/ImGuiLayer.h"
#include "Rod/Project/Project.h"
#include "Rod/Scripting/ScriptEngine.h"
#include "Rod/Utils/PlatformUtils.h"

#include <utility>

namespace Rod {

	EditorLayer::EditorLayer(
	    const Project& project,
	    const Window& window,
	    const Input& input,
	    ImGuiLayer& imGuiLayer,
	    std::function<void()> requestClose)
	    : Layer("Editor Layer"), m_ImGuiLayer(imGuiLayer),
	      m_Scenes(project, EditorViewport::s_DefaultWidth, EditorViewport::s_DefaultHeight),
	      m_Viewport(m_Interaction, input), m_Commands(project, m_Scenes, m_Interaction, FileDialogs{window}),
	      m_Layout(project, m_Scenes, m_Viewport, m_Commands, m_Interaction, window, std::move(requestClose)),
	      m_Input(m_Commands, m_Interaction, input)
	{
		RD_PROFILE_FUNCTION();

		m_Commands.NewScene();
		m_Commands.OpenScene(project.GetStartupScenePath());

		try
		{
			ScriptEngine::Init(project, input, ScriptEngine::Mode::Editor);
		}
		catch (...)
		{
			ScriptEngine::Shutdown();
			throw;
		}
	}

	EditorLayer::~EditorLayer()
	{
		RD_PROFILE_FUNCTION();
		m_Commands.Stop();
		ScriptEngine::Shutdown();
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		ScriptEngine::OnUpdate();
		m_Viewport.OnUpdate(ts, m_Scenes);
		m_LastDeltaTime = ts.GetSeconds();
	}

	void EditorLayer::OnImGuiRender()
	{
		m_Layout.OnImGuiRender(m_LastDeltaTime);
		m_ImGuiLayer.SetBlockEvents(m_Viewport.ShouldBlockEvents());
	}

	void EditorLayer::OnEvent(Event& event)
	{
		m_Viewport.OnEvent(event, m_Scenes.GetState());
		m_Input.OnEvent(event);
	}

}
