#pragma once

#include "Panels/ContentBrowserPanel.h"
#include "Panels/DebugPanel.h"
#include "Panels/GuidePanel.h"
#include "Panels/PerformancePanel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/TitlebarPanel.h"
#include "Panels/ToolbarPanel.h"

#include "imgui.h"

#include <functional>

namespace Rod {

	class Project;
	class EditorCommands;
	class EditorSceneManager;
	class EditorViewport;
	class Window;
	struct EditorInteractionState;

	class EditorLayout
	{
	public:
		EditorLayout(
		    const Project& project,
		    EditorSceneManager& scenes,
		    EditorViewport& viewport,
		    EditorCommands& commands,
		    EditorInteractionState& interaction,
		    const Window& window,
		    std::function<void()> requestClose);
		~EditorLayout() = default;
		EditorLayout(const EditorLayout&) = delete;
		auto operator=(const EditorLayout&) -> EditorLayout& = delete;
		EditorLayout(EditorLayout&&) = delete;
		auto operator=(EditorLayout&&) -> EditorLayout& = delete;

		void OnImGuiRender(float deltaTime);

	private:
		void SetupDefaultDockLayout(ImGuiID dockspaceID);
		void RenderTitlebar() const;
		void RenderDockspace(float deltaTime);
		void RenderPanels(float deltaTime);

		static constexpr float s_LeftDockRatio = 0.230f;
		static constexpr float s_RightDockRatio = 0.299f;
		static constexpr float s_LeftBottomDockRatio = 0.498f;
		static constexpr float s_CenterBottomDockRatio = 0.245f;
		static constexpr float s_ToolbarDockRatio = 0.066f;
		static constexpr float s_RightBottomDockRatio = 0.724f;
		static constexpr float s_RightLowerDockRatio = 0.408f;

		const Project& m_Project;
		EditorSceneManager& m_Scenes;
		EditorViewport& m_Viewport;
		EditorCommands& m_Commands;
		EditorInteractionState& m_Interaction;
		TitlebarPanel m_TitlebarPanel;
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;
		PerformancePanel m_PerformancePanel;
		DebugPanel m_DebugPanel;
		GuidePanel m_GuidePanel;
		ToolbarPanel m_ToolbarPanel;
		bool m_DockLayoutInitialized = false;
		ImGuiID m_ToolbarDockID = 0;
	};

}
