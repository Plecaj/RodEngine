#include "rdpch.h"

#include "EditorLayer.h"

#include "Rod/Scripting/ScriptEngine.h"

namespace Rod {

	EditorLayer::EditorLayer(const Project& project)
	    : Layer("Editor Layer"), m_Scenes(project, EditorViewport::s_DefaultWidth, EditorViewport::s_DefaultHeight),
	      m_Viewport(m_Interaction), m_Commands(project, m_Scenes, m_Interaction),
	      m_Layout(project, m_Scenes, m_Viewport, m_Commands, m_Interaction), m_Input(m_Commands, m_Interaction)
	{
		RD_PROFILE_FUNCTION();

		m_Commands.NewScene();
		m_Commands.OpenScene(project.GetStartupScenePath());

		try
		{
			ScriptEngine::Init();
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
	}

	void EditorLayer::OnEvent(Event& event)
	{
		m_Viewport.OnEvent(event, m_Scenes.GetState());
		m_Input.OnEvent(event);
	}

}