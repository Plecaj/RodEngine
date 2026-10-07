#include "EditorLayer.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuizmo.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>

#include "Rod/Utils/PlatformUtils.h"
#include "Rod/Scripting/ScriptEngine.h"


namespace Rod {

	extern const std::filesystem::path g_AssetsPath;

	void EditorLayer::SetupDefaultDockLayout(ImGuiID dockspaceID)
	{
		ImGuiDockNode* dockNode = ImGui::DockBuilderGetNode(dockspaceID);
		if (dockNode == nullptr)
		{
			return;
		}
		const ImVec2 dockSpaceSize = dockNode->Size;

		ImGui::DockBuilderRemoveNode(dockspaceID);
		ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceID, dockSpaceSize);

		ImGuiID leftDockID, centerDockID, rightDockID;
		ImGuiID leftBottomDockID, centerBottomDockID, toolbarDockID;
		ImGuiID rightBottomDockID, rightBottomLowerDockID;

		leftDockID = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Left, 0.230f, nullptr, &centerDockID);
		rightDockID = ImGui::DockBuilderSplitNode(centerDockID, ImGuiDir_Right, 0.299f, nullptr, &centerDockID);

		leftBottomDockID = ImGui::DockBuilderSplitNode(leftDockID, ImGuiDir_Down, 0.498f, nullptr, &leftDockID);

		centerBottomDockID = ImGui::DockBuilderSplitNode(centerDockID, ImGuiDir_Down, 0.245f, nullptr, &centerDockID);
		toolbarDockID = ImGui::DockBuilderSplitNode(centerDockID, ImGuiDir_Up, 0.066f, nullptr, &centerDockID);
		m_ToolbarDockID = toolbarDockID;

		rightBottomDockID = ImGui::DockBuilderSplitNode(rightDockID, ImGuiDir_Down, 0.724f, nullptr, &rightDockID);
		rightBottomLowerDockID = ImGui::DockBuilderSplitNode(rightBottomDockID, ImGuiDir_Down, 0.408f, nullptr, &rightBottomDockID);

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

	EditorLayer::EditorLayer()
		:Layer("Editor Layer")
	{
	}

	void EditorLayer::OnAttach()
	{
		RD_PROFILE_FUNCTION();

		CreateFramebuffer();
		NewScene();
		DeserializeScene("assets/scenes/Example3D.rod");
		SetupTitlebarCallbacks();
		LoadEditorResources();
		ScriptEngine::Init();
	}

	void EditorLayer::CreateFramebuffer()
	{
		FramebufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::DEPTH24 };
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
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
		m_EditorCamera = EditorCamera(30.0f, 1.778f, 1.0f, 1000.0f);

		m_PlayButton = Texture2D::Create("assets/textures/PlayButton.png");
		m_StopButton = Texture2D::Create("assets/textures/StopButton.png");
	}

	void EditorLayer::OnDetach()
	{
		RD_PROFILE_FUNCTION();
		ScriptEngine::Shutdown();
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		ScriptEngine::OnUpdate();
		ResizeViewportIfNeeded();

		m_Framebuffer->Bind();
		Renderer2D::ResetStats();
		RenderCommand::SetClearColor({ 0.15f, 0.15f, 0.15f, 1 });
		RenderCommand::Clear();
		m_Framebuffer->ClearColorAttachment(1, -1);

		RenderScene(ts);
		UpdateHoveredEntity();

		m_Framebuffer->Unbind();
		m_LastDeltaTime = ts;
	}

	void EditorLayer::ResizeViewportIfNeeded()
	{
		if (m_ViewportSize != m_PendingViewportSize)
		{
			m_ViewportSize = m_PendingViewportSize;
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
		}
	}

	void EditorLayer::RenderScene(Timestep ts)
	{
		switch (m_SceneState)
		{
			case SceneState::Edit:
			{
				m_EditorCamera.OnUpdate(ts);

				m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);
				break;
			}
			case SceneState::Play:
			{
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
		glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		my = viewportSize.y - my;
		int mouseX = (int)mx;
		int mouseY = (int)my;
		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
		{
			int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
			m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_ActiveScene.get());
		}
	}

	void EditorLayer::OnImGuiRender()
	{
		RenderTitlebar();
		RenderDockspace();
	}

	void EditorLayer::RenderTitlebar()
	{
		float titlebarHeight = (float)m_TitlebarPanel.GetHeight();
		const ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, titlebarHeight));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGuiWindowFlags titlebar_flags =
			ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoScrollWithMouse |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoTitleBar;
		ImGui::Begin("Titlebar", nullptr, titlebar_flags);

		m_TitlebarPanel.OnImGuiRender();

		ImGui::End();
		ImGui::PopStyleVar(3);
	}

	void EditorLayer::RenderDockspace()
	{
		static bool dockspaceOpen = true;
		static bool fullscreen = true;
		static bool padding = false;
		static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

		float titlebarHeight = (float)m_TitlebarPanel.GetHeight();
		const ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings;

		ImVec2 dockPos = ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + titlebarHeight);
		ImVec2 dockSize = ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - titlebarHeight);

		ImGui::SetNextWindowPos(dockPos);
		ImGui::SetNextWindowSize(dockSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		if (fullscreen)
		{
			ImGui::SetNextWindowPos(dockPos);
			ImGui::SetNextWindowSize(dockSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspaceFlags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		if (!padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Dockspace", &dockspaceOpen, window_flags);
		if (!padding)
			ImGui::PopStyleVar();
			
		if (fullscreen)
			ImGui::PopStyleVar(2);

		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");

			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspaceFlags);

			if (!m_DockLayoutInitialized)
			{
				SetupDefaultDockLayout(dockspace_id);
				m_DockLayoutInitialized = true;
			}
		}

		style.WindowMinSize.x = 32.0f;

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

		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		m_ViewportPanel.OnImGuiRender(
			m_Framebuffer,
			m_ActiveScene,
			selectedEntity,
			m_EditorCamera,
			m_SceneState,
			m_GizmoType,
			m_ViewportSize,
			m_PendingViewportSize,
			m_ViewportBounds,
			m_ViewportFocused,
			m_ViewportHovered,
			g_AssetsPath,
			[this](const std::filesystem::path& path) { OpenScene(path); }
		);

		m_ToolbarPanel.OnImGuiRender(
			m_SceneState,
			m_PlayButton,
			m_StopButton,
			m_ToolbarDockID,
			[this]() { OnScenePlay(); },
			[this]() { OnSceneStop(); }
		);
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

		bool controlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool shiftPressed = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

		switch (e.GetKeyCode())
		{
			case Key::N:
			{
				if (controlPressed)
					NewScene();

				break;
			}

			case Key::O:
			{
				if (controlPressed)
					OpenScene();

				break;
			}

			case Key::S:
			{
				if (controlPressed && shiftPressed)
					SaveSceneAs();

				else if (controlPressed)
					SaveScene();

				break;
			}

			case Key::Q:
				if(!ImGuizmo::IsUsing())
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
		if (e.GetMouseButton() == Mouse::Button0 && m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt))
		{
			m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
		}
		return false;
	}

	void EditorLayer::NewScene()
	{
		m_EditorScene = CreateRef<Scene>();
		m_ActiveScene = m_EditorScene;
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_SceneOutputFilepath.clear();
	}

	void EditorLayer::OpenScene()
	{
		std::string filepath = FileDialogs::OpenFile("Rod Scene (*.rod)\0*.rod\0");
		if (!filepath.empty())
			OpenScene(filepath);
	}

	void EditorLayer::OpenScene(const std::filesystem::path& path)
	{
		if (DeserializeScene(path))
			m_SceneOutputFilepath = path.string();
	}

	void EditorLayer::SaveScene()
	{
		if (m_SceneOutputFilepath.empty())
			SaveSceneAs();
		if (m_SceneOutputFilepath.empty())
			return;

		SerializeScene(m_SceneOutputFilepath);
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filepath = FileDialogs::SaveFile("Rod Scene (*.rod)\0*.rod\0");
		if (!filepath.empty())
		{
			m_SceneOutputFilepath = filepath;
			SerializeScene(filepath);
		}
	}

	bool EditorLayer::DeserializeScene(const std::filesystem::path& path)
	{
		NewScene();

		SceneSerializer serializer(m_ActiveScene);
		return serializer.DeserializeText(path.string());
	}

	void EditorLayer::SerializeScene(const std::filesystem::path& path)
	{
		SceneSerializer serializer(m_ActiveScene);
		serializer.SerializeText(path.string());
	}

	void EditorLayer::ExportGame()
	{
		Ref<Scene> sceneToExport = m_SceneState == SceneState::Play ? m_EditorScene : m_ActiveScene;
		if (!sceneToExport)
		{
			RD_CORE_ERROR("No scene to export");
			return;
		}

		std::filesystem::path runtimeExecutable = GetRuntimeExecutablePath();
		if (!std::filesystem::exists(runtimeExecutable))
		{
			RD_CORE_ERROR("Runtime not found: {}", runtimeExecutable.string());
			return;
		}

		std::filesystem::path exportRoot = GetExportRootPath();
		std::filesystem::path exportAssets = exportRoot / "assets";

		if (!PrepareExportFolders(exportAssets))
			return;

		if (!CopyRuntimeExecutable(runtimeExecutable, exportRoot))
			return;

		if (!CopyGameAssets(exportAssets))
			return;

		SceneSerializer serializer(sceneToExport);
		serializer.SerializeText((exportAssets / "scenes" / "Startup.rod").string());
		RD_CORE_INFO("Exported game to {}", exportRoot.string());
	}

	std::filesystem::path EditorLayer::GetRuntimeExecutablePath() const
	{
		return std::filesystem::current_path() / "Rod-Runtime" ROD_EXECUTABLE_SUFFIX;
	}

	std::filesystem::path EditorLayer::GetExportRootPath() const
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

	bool EditorLayer::CopyRuntimeExecutable(const std::filesystem::path& runtimeExecutable, const std::filesystem::path& exportRoot)
	{
		std::error_code error;
		std::filesystem::copy_file(runtimeExecutable, exportRoot / "RodGame" ROD_EXECUTABLE_SUFFIX, std::filesystem::copy_options::overwrite_existing, error);
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
		std::filesystem::copy(std::filesystem::current_path() / "assets", exportAssetsPath,
			std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing, error);
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

		m_EditorScene = m_ActiveScene;
		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
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
		m_ActiveScene = m_EditorScene;
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_SceneState = SceneState::Edit;
		m_GizmoType = -1;
		m_EditorCamera.SetControlsEnabled(true);
	}

}
