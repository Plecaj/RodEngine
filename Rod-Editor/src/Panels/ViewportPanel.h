#pragma once

#include "../SceneState.h"
#include "Rod.h"
#include "Rod/Renderer/EditorCamera.h"

#include <array>
#include <cstddef>
#include <filesystem>
#include <functional>

namespace Rod {

	class Scene;
	class SceneHierarchyPanel;

	class ViewportPanel
	{
	public:
		ViewportPanel() = default;

		static constexpr size_t BOUNDS_COUNT = 2;
		struct RenderContext
		{
			const Rod::Framebuffer& Framebuffer;
			Scene& ActiveScene;
			Entity SelectedEntity;
			Rod::EditorCamera& EditorCamera;
			SceneState State;
			int& GizmoType;
			glm::vec2& Size;
			glm::vec2& PendingSize;
			std::array<glm::vec2, BOUNDS_COUNT>& Bounds;
			bool& Focused;
			bool& Hovered;
			const std::filesystem::path& AssetsPath;
			const std::function<void(const std::filesystem::path&)>& OpenScene;
		};
		void OnImGuiRender(const RenderContext& context);

	private:
		static constexpr size_t VECTOR_COMPONENT_COUNT = 3;
		static constexpr float ROTATION_SNAP = 45.0f;
		static constexpr float TRANSLATION_SNAP = 0.5f;
		static std::filesystem::path AcceptSceneDrop(const std::filesystem::path& assetsPath);
		static bool GetCameraMatrices(const RenderContext& context, glm::mat4& view, glm::mat4& projection);
		static void DrawGizmo(const RenderContext& context);
	};

}
