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

		static constexpr size_t s_BoundsCount = 2;
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
			std::array<glm::vec2, s_BoundsCount>& Bounds;
			bool& Focused;
			bool& Hovered;
			const std::filesystem::path& AssetsPath;
			const std::function<void(const std::filesystem::path&)>& OpenScene;
		};
		static void OnImGuiRender(const RenderContext& context);

	private:
		static constexpr size_t s_VectorComponentCount = 3;
		static constexpr float s_RotationSnap = 45.0f;
		static constexpr float s_TranslationSnap = 0.5f;
		[[nodiscard]] static std::filesystem::path AcceptSceneDrop(const std::filesystem::path& assetsPath);
		[[nodiscard]] static bool
		GetCameraMatrices(const RenderContext& context, glm::mat4& view, glm::mat4& projection);
		static void DrawGizmo(const RenderContext& context);
	};

}
