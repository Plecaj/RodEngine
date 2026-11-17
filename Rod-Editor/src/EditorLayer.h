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
		explicit EditorLayer(const Project& project);
		~EditorLayer() override;
		EditorLayer(const EditorLayer&) = delete;
		EditorLayer& operator=(const EditorLayer&) = delete;
		EditorLayer(EditorLayer&&) = delete;
		EditorLayer& operator=(EditorLayer&&) = delete;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;

	private:
		static constexpr float s_LeftDockRatio = 0.230f;
		static constexpr float s_RightDockRatio = 0.299f;
		static constexpr float s_LeftBottomDockRatio = 0.498f;
		static constexpr float s_CenterBottomDockRatio = 0.245f;
		static constexpr float s_ToolbarDockRatio = 0.066f;
		static constexpr float s_RightBottomDockRatio = 0.724f;
		static constexpr float s_RightLowerDockRatio = 0.408f;
		static constexpr float s_EditorFOV = 30.0f;
		static constexpr float s_EditorAspectRatio = 1.778f;
		static constexpr float s_EditorNearClip = 1.0f;
		static constexpr float s_EditorFarClip = 1000.0f;
		static constexpr glm::vec2 s_DefaultViewportSize{1280.0f, 720.0f};
		static constexpr glm::vec4 s_ClearColor{0.15f, 0.15f, 0.15f, 1.0f};

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

		void DeserializeScene(const std::filesystem::path& path);
		void SerializeScene(const std::filesystem::path& path);

	private:
		const Project& m_Project;
		Scene m_EditorScene;
		Scene m_RuntimeScene;
		ObserverPtr<Scene> m_ActiveScene = nullptr;

		EditorCamera m_EditorCamera;

		Scope<Framebuffer> m_Framebuffer;

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = s_DefaultViewportSize;
		glm::vec2 m_PendingViewportSize = s_DefaultViewportSize;
		std::array<glm::vec2, ViewportPanel::s_BoundsCount> m_ViewportBounds{};

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
