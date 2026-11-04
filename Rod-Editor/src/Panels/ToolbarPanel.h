#pragma once

#include "../SceneState.h"
#include "Rod.h"

#include <imgui.h>

#include <functional>

namespace Rod {

	class ToolbarPanel
	{
	public:
		ToolbarPanel() = default;

		void OnImGuiRender(
		    SceneState sceneState,
		    const Texture2D& playButton,
		    const Texture2D& stopButton,
		    ImGuiID dockID,
		    const std::function<void()>& onPlay,
		    const std::function<void()>& onStop);
	};

}
