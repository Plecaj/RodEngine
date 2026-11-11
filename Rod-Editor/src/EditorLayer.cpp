#include "rdpch.h"

#include "EditorLayer.h"

#include "Rod/Scripting/ScriptEngine.h"
#include "Rod/Utils/PlatformUtils.h"

#include "imgui.h"
#include "imgui_internal.h"

#include "ImGuizmo.h"

#include <exception>

namespace Rod {

	void EditorLayer::SetupDefaultDockLayout(ImGuiID dockspaceID)
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

		leftDockID = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Left, LEFT_DOCK_RATIO, nullptr, &centerDockID);
		rightDockID =
		    ImGui::DockBuilderSplitNode(centerDockID, ImGuiDir_Right, RIGHT_DOCK_RATIO, nullptr, &centerDockID);

		leftBottomDockID =
		    ImGui::DockBuilderSplitNode(leftDockID, ImGuiDir_Down, LEFT_BOTTOM_DOCK_RATIO, nullptr, &leftDockID);

		centerBottomDockID =
		    ImGui::DockBuilderSplitNode(centerDockID, ImGuiDir_Down, CENTER_BOTTOM_DOCK_RATIO, nullptr, &centerDockID);
		toolbarDockID =
		    ImGui::DockBuilderSplitNode(centerDockID, ImGuiDir_Up, TOOLBAR_DOCK_RATIO, nullptr, &centerDockID);
		m_ToolbarDockID = toolbarDockID;

		rightBottomDockID =
		    ImGui::DockBuilderSplitNode(rightDockID, ImGuiDir_Down, RIGHT_BOTTOM_DOCK_RATIO, nullptr, &rightDockID);
		rightBottomLowerDockID = ImGui::DockBuilderSplitNode(
		    rightBottomDockID,
		    ImGuiDir_Down,
		    RIGHT_LOWER_DOCK_RATIO,
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
			toolbarNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
			toolbarNode->LocalFlags |= ImGuiDockNodeFlags_NoWindowMenuButton;
			toolbarNode->LocalFlags |= ImGuiDockNodeFlags_NoCloseButton;
			toolbarNode->LocalFlags |= ImGuiDockNodeFlags_NoResize;
			toolbarNode->LocalFlags |= ImGuiDockNodeFlags_NoResizeY;
		}

		if (ImGuiDockNode* viewportNode = ImGui::DockBuilderGetNode(centerDockID))
			viewportNode->LocalFlags |= ImGuiDockNodeFlags_HiddenTabBar;

		ImGui::DockBuilderFinish(dockspaceID);
	}

	EditorLayer::EditorLayer(const Project& project)
	    : Layer("Editor Layer"), m_Project(project),
	      m_SceneHierarchyPanel(project.GetAssetsPath()), m_ContentBrowserPanel(project.GetAssetsPath())
	{}

	void EditorLayer::OnAttach()
	{
		RD_PROFILE_FUNCTION();

		CreateFramebuffer();
		NewScene();
		OpenScene(m_Project.GetStartupScenePath());
		SetupTitlebarCallbacks();
		LoadEditorResources();
		ScriptEngine::Init();
	}

	void EditorLayer::CreateFramebuffer()
	{
		FramebufferSpecification fbSpec;
		fbSpec.Attachments = {
		    FramebufferTextureSpecification{FramebufferTextureFormat::RGBA8},
		    FramebufferTextureSpecification{FramebufferTextureFormat::RedInteger},
		    FramebufferTextureSpecification{FramebufferTextureFormat::DEPTH24}};
		fbSpec.Width = static_cast<uint32_t>(DEFAULT_VIEWPORT_SIZE.x);
		fbSpec.Height = static_cast<uint32_t>(DEFAULT_VIEWPORT_SIZE.y);
		m_Framebuffer = Framebuffer::Create(fbSpec);
	}

	void EditorLayer::SetupTitlebarCallbacks()
	{
		m_TitlebarPanel.SetNewSceneCallback([this]() { NewScene(); });
		m_TitlebarPanel.SetOpenSceneCallback([this]() { OpenScene(); });
		m_TitlebarPanel.SetSaveSceneCallback([this]() { SaveScene(); });
		m_TitlebarPanel.SetSaveSceneAsCallback([this]() { SaveSceneAs(); });
		m_TitlebarPanel.SetExportGameCallback([this]() { ExportGame(); });
	}

	void EditorLayer::LoadEditorResources()
	{
		m_EditorCamera = EditorCamera(EDITOR_FOV, EDITOR_ASPECT_RATIO, EDITOR_NEAR_CLIP, EDITOR_FAR_CLIP);

		m_PlayButton = Texture2D::Create("assets/textures/PlayButton.png");
		m_StopButton = Texture2D::Create("assets/textures/StopButton.png");
	}

	void EditorLayer::OnDetach()
	{
		RD_PROFILE_FUNCTION();
		OnSceneStop();
		ScriptEngine::Shutdown();
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		ScriptEngine::OnUpdate();
		ResizeViewportIfNeeded();

		m_Framebuffer->Bind();
		Renderer2D::ResetStats();
		RenderCommand::SetClearColor(CLEAR_COLOR);
		RenderCommand::Clear();
		m_Framebuffer->ClearColorAttachment(1, -1);

		RenderScene(ts);
		UpdateHoveredEntity();

		m_Framebuffer->Unbind();
		m_LastDeltaTime = ts.GetSeconds();
	}

	void EditorLayer::ResizeViewportIfNeeded()
	{
		if (m_ViewportSize != m_PendingViewportSize)
		{
			m_ViewportSize = m_PendingViewportSize;
			m_Framebuffer->Resize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
			m_ActiveScene->OnViewportResize(
			    static_cast<uint32_t>(m_ViewportSize.x),
			    static_cast<uint32_t>(m_ViewportSize.y));
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
		}
	}

	void EditorLayer::RenderScene(Timestep ts)
	{
		switch (m_SceneState)
		{
			case SceneState::Edit: {
				m_EditorCamera.OnUpdate(ts);

				m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);
				break;
			}
			case SceneState::Play: {
				m_ActiveScene->OnUpdateRuntime(ts);
				break;
			}
		}
	}

	void EditorLayer::UpdateHoveredEntity()
	{
		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		const glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		my = viewportSize.y - my;
		const int mouseX = static_cast<int>(mx);
		const int mouseY = static_cast<int>(my);
		if (mouseX >= 0 && mouseY >= 0 && mouseX < static_cast<int>(viewportSize.x) &&
		    mouseY < static_cast<int>(viewportSize.y))
		{
			const int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
			m_HoveredEntity = pixelData == -1 ? Entity() : Entity(static_cast<entt::entity>(pixelData), m_ActiveScene);
		}
	}

	void EditorLayer::OnImGuiRender()
	{
		RenderTitlebar();
		RenderDockspace();
	}

	void EditorLayer::RenderTitlebar()
	{
		auto titlebarHeight = static_cast<float>(m_TitlebarPanel.GetHeight());
		const ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, titlebarHeight));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		const ImGuiWindowFlags titlebarFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse |
		                                       ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		                                       ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
		                                       ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar;
		ImGui::Begin("Titlebar", nullptr, titlebarFlags);

		m_TitlebarPanel.OnImGuiRender();

		ImGui::End();
		constexpr int titlebarStyleCount = 3;
		ImGui::PopStyleVar(titlebarStyleCount);
	}

	void EditorLayer::RenderDockspace()
	{
		constexpr int fullscreenStyleCount = 2;
		constexpr float dockMinimumWidth = 370.0f;
		constexpr float panelMinimumWidth = 32.0f;
		static bool s_DockspaceOpen = true;
		static const bool FULLSCREEN = true;
		static const bool PADDING = false;
		static ImGuiDockNodeFlags s_DockspaceFlags = ImGuiDockNodeFlags_None;

		auto titlebarHeight = static_cast<float>(m_TitlebarPanel.GetHeight());
		const ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings;

		const ImVec2 dockPos = ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + titlebarHeight);
		const ImVec2 dockSize = ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - titlebarHeight);

		ImGui::SetNextWindowPos(dockPos);
		ImGui::SetNextWindowSize(dockSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		if (FULLSCREEN)
		{
			ImGui::SetNextWindowPos(dockPos);
			ImGui::SetNextWindowSize(dockSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
			               ImGuiWindowFlags_NoMove;
			windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			s_DockspaceFlags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		if (s_DockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
			windowFlags |= ImGuiWindowFlags_NoBackground;

		if (!PADDING)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Dockspace", &s_DockspaceOpen, windowFlags);
		if (!PADDING)
			ImGui::PopStyleVar();

		if (FULLSCREEN)
			ImGui::PopStyleVar(fullscreenStyleCount);

		const ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowMinSize.x = dockMinimumWidth;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
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

		RenderPanels();

		ImGui::End();
	}

	void EditorLayer::RenderPanels()
	{
		m_SceneHierarchyPanel.OnImGuiRender();
		m_ContentBrowserPanel.OnImGuiRender();

		m_PerformancePanel.OnImGuiRender(m_LastDeltaTime);
		m_DebugPanel.OnImGuiRender(m_GizmoType, m_HoveredEntity, m_Profiling);
		m_GuidePanel.OnImGuiRender();

		const Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		m_ViewportPanel.OnImGuiRender(
		    {*m_Framebuffer,
			 *m_ActiveScene,
			 selectedEntity,
			 m_EditorCamera,
			 m_SceneState,
			 m_GizmoType,
			 m_ViewportSize,
			 m_PendingViewportSize,
			 m_ViewportBounds,
			 m_ViewportFocused,
			 m_ViewportHovered,
			 m_Project.GetAssetsPath(),
			 [this](const std::filesystem::path& path) { OpenScene(path); }});

		m_ToolbarPanel.OnImGuiRender(
		    m_SceneState,
		    *m_PlayButton,
		    *m_StopButton,
		    m_ToolbarDockID,
		    [this]() { OnScenePlay(); },
		    [this]() { OnSceneStop(); });
	}

	void EditorLayer::OnEvent(Event& event)
	{
		m_EditorCamera.OnEvent(event);

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(RD_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(RD_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		// Shortcuts
		if (e.IsRepeat())
			return false;

		const bool controlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		const bool shiftPressed = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

		switch (e.GetKeyCode())
		{
			case Key::N: {
				if (controlPressed)
					NewScene();

				break;
			}

			case Key::O: {
				if (controlPressed)
					OpenScene();

				break;
			}

			case Key::S: {
				if (controlPressed && shiftPressed)
					SaveSceneAs();

				else if (controlPressed)
					SaveScene();

				break;
			}

			case Key::Q:
				if (!ImGuizmo::IsUsing())
					m_GizmoType = -1;
				break;
			case Key::W:
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				break;
			case Key::E:
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				break;
			case Key::R:
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::SCALE;
				break;
		}

		return false;
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() == Mouse::Button0 && m_ViewportHovered && !ImGuizmo::IsOver() &&
		    !Input::IsKeyPressed(Key::LeftAlt))
		{
			m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
		}
		return false;
	}

	void EditorLayer::NewScene()
	{
		OnSceneStop();
		m_SceneHierarchyPanel.SetContext(nullptr);
		m_HoveredEntity = {};
		m_EditorScene = {};
		m_ActiveScene = &m_EditorScene;
		m_ActiveScene->OnViewportResize(
		    static_cast<uint32_t>(m_ViewportSize.x),
		    static_cast<uint32_t>(m_ViewportSize.y));
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_SceneOutputFilepath.clear();
	}

	void EditorLayer::OpenScene()
	{
		const std::string filepath = FileDialogs::OpenFile("Rod Scene (*.rod)\0*.rod\0");
		if (!filepath.empty())
			OpenScene(filepath);
	}

	void EditorLayer::OpenScene(const std::filesystem::path& path)
	{
		try
		{
			DeserializeScene(path);
			m_SceneOutputFilepath = path.string();
		}
		catch (const std::exception& exception)
		{
			RD_CORE_ERROR("{}", exception.what());
		}
	}

	void EditorLayer::SaveScene()
	{
		if (m_SceneOutputFilepath.empty())
		{
			SaveSceneAs();
			return;
		}

		try
		{
			SerializeScene(m_SceneOutputFilepath);
		}
		catch (const std::exception& exception)
		{
			RD_CORE_ERROR("{}", exception.what());
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		const std::string filepath = FileDialogs::SaveFile("Rod Scene (*.rod)\0*.rod\0");
		if (!filepath.empty())
		{
			try
			{
				SerializeScene(filepath);
				m_SceneOutputFilepath = filepath;
			}
			catch (const std::exception& exception)
			{
				RD_CORE_ERROR("{}", exception.what());
			}
		}
	}

	void EditorLayer::DeserializeScene(const std::filesystem::path& path)
	{
		Scene loadedScene;
		loadedScene.OnViewportResize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
		SceneSerializer serializer(loadedScene, m_Project.GetDirectory());
		serializer.DeserializeText(path.string());

		NewScene();
		m_EditorScene = std::move(loadedScene);
	}

	void EditorLayer::SerializeScene(const std::filesystem::path& path)
	{
		SceneSerializer serializer(*m_ActiveScene, m_Project.GetDirectory());
		serializer.SerializeText(path.string());
	}

	void EditorLayer::ExportGame()
	{
		Scene* sceneToExport = &m_EditorScene;
		if (!sceneToExport)
		{
			RD_CORE_ERROR("No scene to export");
			return;
		}

		const std::filesystem::path runtimeExecutable = GetRuntimeExecutablePath();
		if (!std::filesystem::exists(runtimeExecutable))
		{
			RD_CORE_ERROR("Runtime not found: {}", runtimeExecutable.string());
			return;
		}

		const std::filesystem::path exportRoot = GetExportRootPath();
		const std::filesystem::path exportAssets = exportRoot / "assets";

		if (!PrepareExportFolders(exportAssets))
			return;

		if (!CopyRuntimeExecutable(runtimeExecutable, exportRoot))
			return;

		if (!CopyGameAssets(exportAssets))
			return;

		SceneSerializer serializer(*sceneToExport);
		try
		{
			serializer.SerializeText((exportAssets / "scenes" / "Startup.rod").string());
		}
		catch (const std::exception& exception)
		{
			RD_CORE_ERROR("{}", exception.what());
			return;
		}
		RD_CORE_INFO("Exported game to {}", exportRoot.string());
	}

	std::filesystem::path EditorLayer::GetRuntimeExecutablePath()
	{
		return std::filesystem::current_path() / "Rod-Runtime" ROD_EXECUTABLE_SUFFIX;
	}

	std::filesystem::path EditorLayer::GetExportRootPath()
	{
		return std::filesystem::current_path() / "exports" / "RodGame";
	}

	bool EditorLayer::PrepareExportFolders(const std::filesystem::path& exportAssetsPath)
	{
		std::error_code error;
		std::filesystem::create_directories(exportAssetsPath / "scenes", error);
		if (error)
		{
			RD_CORE_ERROR("Export folder failed: {}", error.message());
			return false;
		}

		return true;
	}

	bool EditorLayer::CopyRuntimeExecutable(
	    const std::filesystem::path& runtimeExecutable,
	    const std::filesystem::path& exportRoot)
	{
		std::error_code error;
		std::filesystem::copy_file(
		    runtimeExecutable,
		    exportRoot / "RodGame" ROD_EXECUTABLE_SUFFIX,
		    std::filesystem::copy_options::overwrite_existing,
		    error);
		if (error)
		{
			RD_CORE_ERROR("Runtime copy failed: {}", error.message());
			return false;
		}

		return true;
	}

	bool EditorLayer::CopyGameAssets(const std::filesystem::path& exportAssetsPath)
	{
		std::error_code error;
		std::filesystem::copy(
		    std::filesystem::current_path() / "assets",
		    exportAssetsPath,
		    std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing,
		    error);
		if (error)
		{
			RD_CORE_ERROR("Asset copy failed: {}", error.message());
			return false;
		}

		return true;
	}

	void EditorLayer::OnScenePlay()
	{
		if (m_SceneState != SceneState::Edit)
			return;

		m_RuntimeScene = m_EditorScene;
		m_ActiveScene = &m_RuntimeScene;
		m_HoveredEntity = {};
		m_ActiveScene->OnViewportResize(
		    static_cast<uint32_t>(m_ViewportSize.x),
		    static_cast<uint32_t>(m_ViewportSize.y));
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		m_SceneState = SceneState::Play;
		m_GizmoType = -1;
		m_EditorCamera.SetControlsEnabled(false);
		m_ActiveScene->OnRuntimeStart();
	}

	void EditorLayer::OnSceneStop()
	{
		if (m_SceneState != SceneState::Play)
			return;

		m_ActiveScene->OnRuntimeStop();
		m_ActiveScene = &m_EditorScene;
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_HoveredEntity = {};
		m_SceneState = SceneState::Edit;
		m_GizmoType = -1;
		m_EditorCamera.SetControlsEnabled(true);
	}

} // namespace Rod
