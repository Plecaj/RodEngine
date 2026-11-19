#pragma once

#include "../SceneState.h"
#include "Rod.h"

#include <imgui.h>

#include <functional>

namespace Rod {

	class ToolbarPanel
	{
	public:
		ToolbarPanel();

		struct Actions
		{
			std::function<void()> Play;
			std::function<void()> Stop;
		};
		void OnImGuiRender(SceneState sceneState, ImGuiID dockID, const Actions& actions) const;

	private:
		Scope<Texture2D> m_PlayButton, m_StopButton;
	};

}
