#pragma once

#include "../SceneState.h"
#include "Rod/Renderer/EditorCamera.h"
#include "Rod/Renderer/FrameBuffer.h"
#include "Rod/Scene/Entity.h"

#include <array>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <optional>

namespace Rod {

	class InputState;
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
			SceneState State = SceneState::Edit;
			int& GizmoType;
			glm::vec2& Size;
			glm::vec2& PendingSize;
			std::array<glm::vec2, s_BoundsCount>& Bounds;
			bool& Focused;
			bool& Hovered;
			const std::filesystem::path& AssetsPath;
			const std::function<void(const std::filesystem::path&)>& OpenScene;
			const InputState& WindowInput;
		};
		static void OnImGuiRender(const RenderContext& context);

	private:
		static constexpr size_t s_VectorComponentCount = 3;
		static constexpr float s_RotationSnap = 45.0f;
		static constexpr float s_TranslationSnap = 0.5f;
		struct CameraMatrices
		{
			glm::mat4 View{};
			glm::mat4 Projection{};
		};
		[[nodiscard]] static auto AcceptSceneDrop(const std::filesystem::path& assetsPath) -> std::filesystem::path;
		[[nodiscard]] static auto GetCameraMatrices(const RenderContext& context) -> std::optional<CameraMatrices>;
		static void DrawGizmo(const RenderContext& context);
	};

}
