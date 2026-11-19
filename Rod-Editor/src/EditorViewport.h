#pragma once

#include "Panels/ViewportPanel.h"
#include "Rod/Renderer/EditorCamera.h"

namespace Rod {

	class EditorSceneManager;
	class Input;
	struct EditorInteractionState;

	class EditorViewport
	{
	public:
		static constexpr uint32_t s_DefaultWidth = 1280;
		static constexpr uint32_t s_DefaultHeight = 720;

		EditorViewport(EditorInteractionState& interaction, const Input& input);

		void OnUpdate(Timestep ts, EditorSceneManager& scenes);
		void OnEvent(Event& event, SceneState state);
		void OnImGuiRender(
		    EditorSceneManager& scenes,
		    const std::filesystem::path& assetsPath,
		    const std::function<void(const std::filesystem::path&)>& openScene);
		[[nodiscard]] bool ShouldBlockEvents() const noexcept;

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
		void UpdateHoveredEntity(EditorSceneManager& scenes) const;

		EditorInteractionState& m_Interaction;
		const Input& m_Input;
		EditorCamera m_EditorCamera;
		Scope<Framebuffer> m_Framebuffer;
		bool m_ViewportFocused = false;
		glm::vec2 m_ViewportSize = s_DefaultViewportSize;
		glm::vec2 m_PendingViewportSize = s_DefaultViewportSize;
		std::array<glm::vec2, ViewportPanel::s_BoundsCount> m_ViewportBounds{};
	};

}
