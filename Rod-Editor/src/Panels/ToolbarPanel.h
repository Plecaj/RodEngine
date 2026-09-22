#pragma once

#include "Rod.h"
#include <imgui.h>
#include <functional>

#include "../SceneState.h"

namespace Rod {

	class ToolbarPanel {
	public:
		ToolbarPanel() = default;
		~ToolbarPanel() = default;

		void OnImGuiRender(SceneState sceneState, Ref<Texture2D> playButton, Ref<Texture2D> stopButton, ImGuiID dockID,
			std::function<void()> onPlay, std::function<void()> onStop);
	};

}
