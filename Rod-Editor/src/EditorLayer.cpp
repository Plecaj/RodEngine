#include "EditorLayer.h"
#include "imgui.h"
#include "ImGuizmo.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>

#include "Rod/Utils/PlatformUtils.h"


namespace Rod {

	// Temporary 
	extern const std::filesystem::path g_AssetsPath;

	EditorLayer::EditorLayer()
		:Layer("Sandbox2D")
	{
	}

	void EditorLayer::OnAttach()
	{
		RD_PROFILE_FUNCTION();

		FramebufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		NewScene();
		SceneSerializer serializer(m_ActiveScene);
		serializer.DeserializeText("assets/scenes/Example.rod");

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		m_TitlebarPanel.SetNewSceneCallback([this]() { NewScene(); });
		m_TitlebarPanel.SetOpenSceneCallback([this]() { OpenScene(); });
		m_TitlebarPanel.SetSaveSceneCallback([this]() { SaveScene(); });
		m_TitlebarPanel.SetSaveSceneAsCallback([this]() { SaveSceneAs(); });

		m_EditorCamera = EditorCamera(30.0f, 1.778f, 1.0f, 1000.0f);

		m_PlayButton = Texture2D::Create("assets/textures/PlayButton.png");
		m_StopButton = Texture2D::Create("assets/textures/StopButton.png");
	}

	void EditorLayer::OnDetach()
	{
		RD_PROFILE_FUNCTION();
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		if (m_ViewportSize != m_PendingViewportSize)
		{
			m_ViewportSize = m_PendingViewportSize;
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_EditorCamera.SetViewportSize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		m_EditorCamera.OnUpdate(ts);

		m_Framebuffer->Bind();
		Renderer2D::ResetStats();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();

		m_Framebuffer->ClearColorAttachment(1, -1);

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

		m_Framebuffer->Unbind();

		m_LastDeltaTime = ts;
	}

	void EditorLayer::OnImGuiRender()
	{
		float titlebarHeight = (float)m_TitlebarPanel.GetHeight() - 20.0f;
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
			ImGuiWindowFlags_NoTitleBar;
		ImGui::Begin("Titlebar", nullptr, titlebar_flags);

		m_TitlebarPanel.OnImGuiRender();

		ImGui::End();
		ImGui::PopStyleVar(3);

		// Dockspace code from ImGui demo
		static bool dockspaceOpen = true;
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

		ImVec2 dockPos = ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + titlebarHeight);
		ImVec2 dockSize = ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - titlebarHeight);

		ImGui::SetNextWindowPos(dockPos);
		ImGui::SetNextWindowSize(dockSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Dockspace", &dockspaceOpen, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();
			
		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");

			ImGui::GetStyle().Colors[ImGuiCol_Separator].w = 0.0f;
			ImGui::GetStyle().Colors[ImGuiCol_DockingPreview].w = 0.0f;

			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = 32.0f;

		m_SceneHierarchyPanel.OnImGuiRender();
		m_ContentBrowserPanel.OnImGuiRender();

		m_PerformancePanel.OnImGuiRender(m_LastDeltaTime);
		m_DebugPanel.OnImGuiRender(m_GuizmoType, m_HoveredEntity, m_Profiling);
		m_GuidePanel.OnImGuiRender();

		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		m_ViewportPanel.OnImGuiRender(
			m_Framebuffer,
			m_ActiveScene,
			selectedEntity,
			m_EditorCamera,
			m_SceneState,
			m_GuizmoType,
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
			[this]() { OnScenePlay(); },
			[this]() { OnSceneStop(); }
		);

		ImGui::End();
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

			// Guizmos
			case Key::Q:
				if(!ImGuizmo::IsUsing())
					m_GuizmoType = -1;
				break;
			case Key::W:
				if (!ImGuizmo::IsUsing())
					m_GuizmoType = ImGuizmo::OPERATION::TRANSLATE;
				break;
			case Key::E:
				if (!ImGuizmo::IsUsing())
					m_GuizmoType = ImGuizmo::OPERATION::ROTATE;
				break;
			case Key::R:
				if (!ImGuizmo::IsUsing())
					m_GuizmoType = ImGuizmo::OPERATION::SCALE;
				break;
		}
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
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OpenScene()
	{
		std::string filepath = FileDialogs::OpenFile("Rod Scene (*.rod)\0*.rod\0");
		if (!filepath.empty())
		{
			NewScene();

			SceneSerializer serializer(m_ActiveScene);
			serializer.DeserializeText(filepath);
		}
	}

	void EditorLayer::OpenScene(const std::filesystem::path& path)
	{
		NewScene();

		SceneSerializer serializer(m_ActiveScene);
		serializer.DeserializeText(path.string());
	}

	void EditorLayer::SaveScene()
	{
		if (m_SceneOutputFilepath.empty())
			SaveSceneAs();

		SceneSerializer serializer(m_ActiveScene);
		serializer.SerializeText(m_SceneOutputFilepath);
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filepath = FileDialogs::SaveFile("Rod Scene (*.rod)\0*.rod\0");
		m_SceneOutputFilepath = filepath;
		if (!filepath.empty())
		{
			SceneSerializer serializer(m_ActiveScene);
			serializer.SerializeText(filepath);
		}
	}

	void EditorLayer::OnScenePlay()
	{
		m_SceneState = SceneState::Play;
		m_GuizmoType = -1;
		m_EditorCamera.SetControlsEnabled(false);
	}

	void EditorLayer::OnSceneStop()
	{
		m_SceneState = SceneState::Edit;
		m_GuizmoType = -1;
		m_EditorCamera.SetControlsEnabled(true);
	}

}