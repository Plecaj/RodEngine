#include "rdpch.h"

#include "EditorLayout.h"

#include "EditorCommands.h"
#include "EditorInteractionState.h"
#include "EditorSceneManager.h"
#include "EditorViewport.h"
#include "Rod/ImGui/ImGuiFlags.h"
#include "Rod/Project/Project.h"

#include "imgui_internal.h"

#include <utility>

namespace Rod {

	EditorLayout::EditorLayout(
	    const Project& project,
	    EditorSceneManager& scenes,
	    EditorViewport& viewport,
	    EditorCommands& commands,
	    EditorInteractionState& interaction,
	    const Window& window,
	    const RenderDevice& device,
	    std::function<void()> requestClose)
	    : m_Project(project), m_Scenes(scenes), m_Viewport(viewport), m_Commands(commands), m_Interaction(interaction),
	      m_TitlebarPanel(commands, window, device, std::move(requestClose)),
	      m_SceneHierarchyPanel(project.GetAssetsPath(), interaction, device),
	      m_ContentBrowserPanel(project.GetAssetsPath(), device), m_ToolbarPanel(device)
	{}

	void EditorLayout::OnImGuiRender(float deltaTime)
	{
		RenderTitlebar();
		RenderDockspace(deltaTime);
	}

	void EditorLayout::RenderPanels(float deltaTime)
	{
		m_SceneHierarchyPanel.OnImGuiRender(m_Scenes.GetActiveScene());
		m_ContentBrowserPanel.OnImGuiRender();
		m_PerformancePanel.OnImGuiRender(deltaTime);
		m_DebugPanel.OnImGuiRender(m_Interaction.GizmoType, m_Interaction.HoveredEntity);
		m_GuidePanel.OnImGuiRender();

		m_Viewport.OnImGuiRender(m_Scenes, m_Project.GetAssetsPath(), [this](const std::filesystem::path& path) {
			m_Commands.OpenScene(path);
		});
		m_ToolbarPanel.OnImGuiRender(
		    m_Scenes.GetState(),
		    m_ToolbarDockID,
		    {.Play = [this] { m_Commands.Play(); }, .Stop = [this] { m_Commands.Stop(); }});
	}

	void EditorLayout::SetupDefaultDockLayout(ImGuiID dockspaceID)
	{
		const ImGuiDockNode* dockNode = ImGui::DockBuilderGetNode(dockspaceID);
		if (dockNode == nullptr)
		{
			return;
		}
		const ImVec2 dockSpaceSize = dockNode->Size;

		ImGui::DockBuilderRemoveNode(dockspaceID);
		ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceID, dockSpaceSize);

		ImGuiID leftDockID{}, centerDockID{}, rightDockID{};
		ImGuiID leftBottomDockID{}, centerBottomDockID{}, toolbarDockID{};
		ImGuiID rightBottomDockID{}, rightBottomLowerDockID{};

		leftDockID = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Left, s_LeftDockRatio, nullptr, &centerDockID);
		rightDockID =
		    ImGui::DockBuilderSplitNode(centerDockID, ImGuiDir_Right, s_RightDockRatio, nullptr, &centerDockID);

		leftBottomDockID =
		    ImGui::DockBuilderSplitNode(leftDockID, ImGuiDir_Down, s_LeftBottomDockRatio, nullptr, &leftDockID);

		centerBottomDockID =
		    ImGui::DockBuilderSplitNode(centerDockID, ImGuiDir_Down, s_CenterBottomDockRatio, nullptr, &centerDockID);
		toolbarDockID =
		    ImGui::DockBuilderSplitNode(centerDockID, ImGuiDir_Up, s_ToolbarDockRatio, nullptr, &centerDockID);
		m_ToolbarDockID = toolbarDockID;

		rightBottomDockID =
		    ImGui::DockBuilderSplitNode(rightDockID, ImGuiDir_Down, s_RightBottomDockRatio, nullptr, &rightDockID);
		rightBottomLowerDockID = ImGui::DockBuilderSplitNode(
		    rightBottomDockID,
		    ImGuiDir_Down,
		    s_RightLowerDockRatio,
		    nullptr,
		    &rightBottomDockID);

		ImGui::DockBuilderDockWindow("Scene Hierarchy", leftDockID);
		ImGui::DockBuilderDockWindow("Properties", leftBottomDockID);

		ImGui::DockBuilderDockWindow("##toolbar", toolbarDockID);
		ImGui::DockBuilderDockWindow("Viewport", centerDockID);
		ImGui::DockBuilderDockWindow("Content Browser", centerBottomDockID);

		ImGui::DockBuilderDockWindow("Performance", rightDockID);
		ImGui::DockBuilderDockWindow("Other", rightBottomDockID);
		ImGui::DockBuilderDockWindow("Guide", rightBottomLowerDockID);

		if (ImGuiDockNode* toolbarNode = ImGui::DockBuilderGetNode(toolbarDockID))
		{
			toolbarNode->LocalFlags = CombineImGuiFlags(
			    toolbarNode->LocalFlags,
			    ImGuiDockNodeFlags_NoTabBar,
			    ImGuiDockNodeFlags_NoWindowMenuButton,
			    ImGuiDockNodeFlags_NoCloseButton,
			    ImGuiDockNodeFlags_NoResize,
			    ImGuiDockNodeFlags_NoResizeY);
		}

		if (ImGuiDockNode* viewportNode = ImGui::DockBuilderGetNode(centerDockID))
			viewportNode->LocalFlags = CombineImGuiFlags(viewportNode->LocalFlags, ImGuiDockNodeFlags_HiddenTabBar);

		ImGui::DockBuilderFinish(dockspaceID);
	}

	void EditorLayout::RenderTitlebar() const
	{
		const auto titlebarHeight = static_cast<float>(m_TitlebarPanel.GetHeight());
		const ImGuiViewport& viewport = *ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(viewport.WorkPos);
		ImGui::SetNextWindowSize(ImVec2(viewport.WorkSize.x, titlebarHeight));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		const ImGuiWindowFlags titlebarFlags = CombineImGuiFlags(
		    ImGuiWindowFlags_NoDocking,
		    ImGuiWindowFlags_NoCollapse,
		    ImGuiWindowFlags_NoResize,
		    ImGuiWindowFlags_NoMove,
		    ImGuiWindowFlags_NoScrollbar,
		    ImGuiWindowFlags_NoScrollWithMouse,
		    ImGuiWindowFlags_NoSavedSettings,
		    ImGuiWindowFlags_NoTitleBar);
		ImGui::Begin("Titlebar", nullptr, titlebarFlags);

		m_TitlebarPanel.OnImGuiRender();

		ImGui::End();
		constexpr int titlebarStyleCount = 3;
		ImGui::PopStyleVar(titlebarStyleCount);
	}

	void EditorLayout::RenderDockspace(float deltaTime)
	{
		constexpr int fullscreenStyleCount = 2;
		constexpr float dockMinimumWidth = 370.0f;
		constexpr float panelMinimumWidth = 32.0f;
		static bool s_DockspaceOpen = true;
		static const bool fullscreen = true;
		static const bool padding = false;
		static ImGuiDockNodeFlags s_DockspaceFlags = ImGuiDockNodeFlags_None;

		const auto titlebarHeight = static_cast<float>(m_TitlebarPanel.GetHeight());
		const ImGuiViewport& viewport = *ImGui::GetMainViewport();

		ImGuiWindowFlags windowFlags = CombineImGuiFlags(ImGuiWindowFlags_NoDocking, ImGuiWindowFlags_NoSavedSettings);

		const ImVec2 dockPos = ImVec2(viewport.WorkPos.x, viewport.WorkPos.y + titlebarHeight);
		const ImVec2 dockSize = ImVec2(viewport.WorkSize.x, viewport.WorkSize.y - titlebarHeight);

		ImGui::SetNextWindowPos(dockPos);
		ImGui::SetNextWindowSize(dockSize);
		ImGui::SetNextWindowViewport(viewport.ID);

		if (fullscreen)
		{
			ImGui::SetNextWindowPos(dockPos);
			ImGui::SetNextWindowSize(dockSize);
			ImGui::SetNextWindowViewport(viewport.ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			windowFlags = CombineImGuiFlags(
			    windowFlags,
			    ImGuiWindowFlags_NoTitleBar,
			    ImGuiWindowFlags_NoCollapse,
			    ImGuiWindowFlags_NoResize,
			    ImGuiWindowFlags_NoMove,
			    ImGuiWindowFlags_NoBringToFrontOnFocus,
			    ImGuiWindowFlags_NoNavFocus);
		}
		else
		{
			s_DockspaceFlags = WithoutImGuiFlags(s_DockspaceFlags, ImGuiDockNodeFlags_PassthruCentralNode);
		}

		if (HasAnyImGuiFlag(s_DockspaceFlags, ImGuiDockNodeFlags_PassthruCentralNode))
			windowFlags = CombineImGuiFlags(windowFlags, ImGuiWindowFlags_NoBackground);

		if (!padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Dockspace", &s_DockspaceOpen, windowFlags);
		if (!padding)
			ImGui::PopStyleVar();

		if (fullscreen)
			ImGui::PopStyleVar(fullscreenStyleCount);

		const ImGuiIO& imguiIO = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowMinSize.x = dockMinimumWidth;
		if (HasAnyImGuiFlag(imguiIO.ConfigFlags, ImGuiConfigFlags_DockingEnable))
		{
			const ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");

			ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), s_DockspaceFlags);

			if (!m_DockLayoutInitialized)
			{
				SetupDefaultDockLayout(dockspaceId);
				m_DockLayoutInitialized = true;
			}
		}

		style.WindowMinSize.x = panelMinimumWidth;

		RenderPanels(deltaTime);

		ImGui::End();
	}

} // namespace Rod
