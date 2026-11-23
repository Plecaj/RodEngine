#pragma once

#include "Panels/ViewportPanel.h"
#include "Rod/Core/Timestep.h"
#include "Rod/Renderer/EditorCamera.h"

#include <optional>

namespace Rod {

	class EditorSceneManager;
	class InputState;
	struct MouseButtonChanged;
	struct Scrolled;
	struct EditorInteractionState;

	class EditorViewport
	{
	public:
		static constexpr uint32_t s_DefaultWidth = 1280;
		static constexpr uint32_t s_DefaultHeight = 720;

		EditorViewport(EditorInteractionState& interaction, const InputState& input);

		void OnUpdate(Timestep ts, EditorSceneManager& scenes);
		void HandleButton(const MouseButtonChanged& button, EditorSceneManager& scenes);
		void HandleScroll(const Scrolled& scroll, SceneState state);
		void OnImGuiRender(
		    EditorSceneManager& scenes,
		    const std::filesystem::path& assetsPath,
		    const std::function<void(const std::filesystem::path&)>& openScene);
		[[nodiscard]] auto HasFocus() const noexcept -> bool { return m_ViewportFocused; }

	private:
		static constexpr float s_EditorFOV = 30.0f;
		static constexpr float s_EditorAspectRatio = 1.778f;
		static constexpr float s_EditorNearClip = 1.0f;
		static constexpr float s_EditorFarClip = 1000.0f;
		static constexpr glm::vec2 s_DefaultViewportSize{s_DefaultWidth, s_DefaultHeight};
		static constexpr glm::vec4 s_ClearColor{0.15f, 0.15f, 0.15f, 1.0f};

		void CreateFramebuffer();
		void ResizeViewportIfNeeded(EditorSceneManager& scenes);
		void RenderScene(Timestep ts, EditorSceneManager& scenes);
		void UpdateCamera();
		[[nodiscard]] auto Contains(glm::vec2 pos) const noexcept -> bool;
		[[nodiscard]] auto Pick(glm::vec2 pos, Scene& scene) const -> Entity;

		EditorInteractionState& m_Interaction;
		const InputState& m_Input;
		EditorCamera m_EditorCamera;
		Scope<Framebuffer> m_Framebuffer;
		std::optional<glm::vec2> m_DragStart;
		bool m_Dragging = false;
		bool m_ViewportFocused = false;
		glm::vec2 m_ViewportSize = s_DefaultViewportSize;
		glm::vec2 m_PendingViewportSize = s_DefaultViewportSize;
		std::array<glm::vec2, ViewportPanel::s_BoundsCount> m_ViewportBounds{};
	};

}
