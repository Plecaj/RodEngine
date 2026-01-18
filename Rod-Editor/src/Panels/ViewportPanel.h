#pragma once

#include "Rod.h"
#include "Rod/Renderer/EditorCamera.h"
#include "../SceneState.h"

namespace Rod {

	class Scene;
	class SceneHierarchyPanel;

	class ViewportPanel {
	public:
		ViewportPanel() = default;
		~ViewportPanel() = default;

		void OnImGuiRender(
			Ref<Framebuffer> framebuffer,
			Ref<Scene> activeScene,
			Entity selectedEntity,
			EditorCamera& editorCamera,
			SceneState sceneState,
			int& guizmoType,
			glm::vec2& viewportSize,
			glm::vec2& pendingViewportSize,
			glm::vec2 viewportBounds[2],
			bool& viewportFocused,
			bool& viewportHovered,
			const std::filesystem::path& assetsPath,
			std::function<void(const std::filesystem::path&)> onOpenScene
		);
	};

}
