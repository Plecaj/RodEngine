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

		void OnImGuiRender(
		    SceneState sceneState,
		    ImGuiID dockID,
		    const std::function<void()>& onPlay,
		    const std::function<void()>& onStop) const;

	private:
		Scope<Texture2D> m_PlayButton, m_StopButton;
	};

}
