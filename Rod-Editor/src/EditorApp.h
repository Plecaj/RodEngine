#pragma once

#include "EditorCommands.h"
#include "EditorInput.h"
#include "EditorInteractionState.h"
#include "EditorLayout.h"
#include "EditorSceneManager.h"
#include "EditorViewport.h"

namespace Rod {

	class DesktopHost;
	class Project;
	struct Frame;

	class EditorApp
	{
	public:
		EditorApp(const Project& project, DesktopHost& host);
		~EditorApp();
		EditorApp(const EditorApp&) = delete;
		auto operator=(const EditorApp&) -> EditorApp& = delete;
		EditorApp(EditorApp&&) = delete;
		auto operator=(EditorApp&&) -> EditorApp& = delete;

		void Update(const Frame& frame);

	private:
		void HandleInput(const Frame& frame);

		EditorSceneManager m_Scenes;
		EditorInteractionState m_Interaction;
		EditorViewport m_Viewport;
		EditorCommands m_Commands;
		EditorLayout m_Layout;
		EditorInput m_Input;
	};

}
