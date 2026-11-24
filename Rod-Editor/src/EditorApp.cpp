#include "rdpch.h"

#include "EditorApp.h"

#include "Rod/Core/DesktopHost.h"
#include "Rod/ImGui/ImGuiFlags.h"
#include "Rod/Project/Project.h"
#include "Rod/Scripting/ScriptEngine.h"
#include "Rod/Utils/PlatformUtils.h"

#include "imgui.h"

namespace Rod {

	EditorApp::EditorApp(const Project& project, DesktopHost& host)
	    : m_Scenes(project, EditorViewport::s_DefaultWidth, EditorViewport::s_DefaultHeight),
	      m_Viewport(m_Interaction, host.GetInput()),
	      m_Commands(project, m_Scenes, m_Interaction, FileDialogs{host.GetWindow()}),
	      m_Layout(
	          project,
	          m_Scenes,
	          m_Viewport,
	          m_Commands,
	          m_Interaction,
	          host.GetWindow(),
	          [&host] { host.RequestClose(); }),
	      m_Input(m_Commands, m_Interaction)
	{
		m_Commands.NewScene();
		m_Commands.OpenScene(project.GetStartupScenePath());

		try
		{
			ScriptEngine::Init(project, host.GetInput(), ScriptEngine::Mode::Editor);
		}
		catch (...)
		{
			ScriptEngine::Shutdown();
			throw;
		}
	}

	EditorApp::~EditorApp()
	{
		m_Commands.Stop();
		ScriptEngine::Shutdown();
	}

	void EditorApp::Update(const Frame& frame)
	{
		HandleInput(frame);
		if (frame.Drawable)
		{
			ScriptEngine::OnUpdate();
			m_Viewport.ResizeViewportIfNeeded(m_Scenes);
			if (m_Scenes.GetState() == SceneState::Play)
				ScriptEngine::OnRuntimeUpdate(frame.Delta);
			m_Viewport.OnUpdate(m_Scenes);
		}
		m_Layout.OnImGuiRender(frame.Delta.GetSeconds());
	}

	void EditorApp::HandleInput(const Frame& frame)
	{
		const auto popupFlags = CombineImGuiFlags(ImGuiPopupFlags_AnyPopupId, ImGuiPopupFlags_AnyPopupLevel);
		const bool popupOpen = ImGui::IsPopupOpen(nullptr, popupFlags);
		const ImGuiIO& io = ImGui::GetIO();
		const bool shortcuts = !popupOpen && !io.WantTextInput &&
		                       (!io.WantCaptureKeyboard || (m_Viewport.HasFocus() && !ImGui::IsAnyItemActive()));

		for (const Event& event : frame.Events)
		{
			if (const auto* key = std::get_if<KeyChanged>(&event))
			{
				if (shortcuts && key->Action == KeyAction::Press)
					m_Input.HandleKey(*key);
			}
			else if (!popupOpen)
			{
				if (const auto* click = std::get_if<MouseButtonChanged>(&event))
					m_Viewport.HandleButton(*click, m_Scenes);
				else if (const auto* scroll = std::get_if<Scrolled>(&event))
					m_Viewport.HandleScroll(*scroll, m_Scenes.GetState());
			}
		}
	}

}
