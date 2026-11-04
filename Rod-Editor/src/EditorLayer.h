#pragma once

#include "Panels/ContentBrowserPanel.h"
#include "Panels/DebugPanel.h"
#include "Panels/GuidePanel.h"
#include "Panels/PerformancePanel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/TitlebarPanel.h"
#include "Panels/ToolbarPanel.h"
#include "Panels/ViewportPanel.h"
#include "Rod.h"
#include "Rod/Renderer/EditorCamera.h"
#include "SceneState.h"

#include "imgui.h"

namespace Rod {

	class EditorLayer : public Rod::Layer
	{
	public:
		EditorLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;

	private:
		static constexpr float LEFT_DOCK_RATIO = 0.230f;
		static constexpr float RIGHT_DOCK_RATIO = 0.299f;
		static constexpr float LEFT_BOTTOM_DOCK_RATIO = 0.498f;
		static constexpr float CENTER_BOTTOM_DOCK_RATIO = 0.245f;
		static constexpr float TOOLBAR_DOCK_RATIO = 0.066f;
		static constexpr float RIGHT_BOTTOM_DOCK_RATIO = 0.724f;
		static constexpr float RIGHT_LOWER_DOCK_RATIO = 0.408f;
		static constexpr float EDITOR_FOV = 30.0f;
		static constexpr float EDITOR_ASPECT_RATIO = 1.778f;
		static constexpr float EDITOR_NEAR_CLIP = 1.0f;
		static constexpr float EDITOR_FAR_CLIP = 1000.0f;
		static constexpr glm::vec2 DEFAULT_VIEWPORT_SIZE{1280.0f, 720.0f};
		static constexpr glm::vec4 CLEAR_COLOR{0.15f, 0.15f, 0.15f, 1.0f};

		void SetupDefaultDockLayout(ImGuiID dockspaceID);

		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveScene();
		void SaveSceneAs();
		void ExportGame();

		void OnScenePlay();
		void OnSceneStop();

		void CreateFramebuffer();
		void LoadEditorResources();
		void SetupTitlebarCallbacks();

		void ResizeViewportIfNeeded();
		void RenderScene(Timestep ts);
		void UpdateHoveredEntity();

		void RenderTitlebar();
		void RenderDockspace();
		void RenderPanels();

		bool DeserializeScene(const std::filesystem::path& path);
		void SerializeScene(const std::filesystem::path& path);

		[[nodiscard]] static std::filesystem::path GetRuntimeExecutablePath();
		[[nodiscard]] static std::filesystem::path GetExportRootPath();
		bool PrepareExportFolders(const std::filesystem::path& exportAssetsPath);
		bool
		CopyRuntimeExecutable(const std::filesystem::path& runtimeExecutable, const std::filesystem::path& exportRoot);
		bool CopyGameAssets(const std::filesystem::path& exportAssetsPath);

	private:
		Scene m_EditorScene;
		Scene m_RuntimeScene;
		Scene* m_ActiveScene = nullptr;

		EditorCamera m_EditorCamera;

		Scope<Framebuffer> m_Framebuffer;

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = DEFAULT_VIEWPORT_SIZE;
		glm::vec2 m_PendingViewportSize = DEFAULT_VIEWPORT_SIZE;
		std::array<glm::vec2, ViewportPanel::BOUNDS_COUNT> m_ViewportBounds{};

		bool m_Profiling = false;

		std::string m_SceneOutputFilepath;

		Entity m_HoveredEntity;

		int m_GizmoType = -1;

		float m_LastDeltaTime = 0.0f;

		SceneState m_SceneState = SceneState::Edit;

		Scope<Texture2D> m_PlayButton, m_StopButton;
		bool m_DockLayoutInitialized = false;
		ImGuiID m_ToolbarDockID = 0;

	private:
		TitlebarPanel m_TitlebarPanel;
		friend class TitlebarPanel;
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;
		PerformancePanel m_PerformancePanel;
		DebugPanel m_DebugPanel;
		GuidePanel m_GuidePanel;
		ViewportPanel m_ViewportPanel;
		ToolbarPanel m_ToolbarPanel;
	};

}
