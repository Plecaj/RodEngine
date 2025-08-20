#include "EditorLayer.h"
#include "imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>

#include "Rod/Utils/PlatformUtils.h"

#include "ImGuizmo.h"
#include "Rod/Math/Math.h"

namespace Rod {

	EditorLayer::EditorLayer()
		:Layer("Sandbox2D"), m_CameraController(1280.0f, 720.0f)
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

		if (m_ViewportFocused)
		{
			m_CameraController.OnUpdate(ts);
			m_EditorCamera.OnUpdate(ts);
		}

		m_EditorCamera.OnUpdate(ts);

		m_Framebuffer->Bind();
		Renderer2D::ResetStats();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();

		m_Framebuffer->ClearColorAttachment(1, -1);

		m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);


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
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = 32.0f;

		m_SceneHierarchyPanel.OnImGuiRender();
		m_ContentBrowserPanel.OnImGuiRender();

		ImGui::Begin("Performance");

			float fps = 1.0f / m_LastDeltaTime;
			ImGui::Text("FPS: %.1f", fps);
			ImGui::Text("Frame Time: %.3f ms", m_LastDeltaTime * 1000.0f);

			ImGui::Separator();

			auto stats = Renderer2D::GetStats();
			ImGui::Text("Renderer Stats:");
			ImGui::Text("	Draw calls: %d", stats.DrawCalls);
			ImGui::Text("	Quad Count: %d", stats.QuadCount);

		ImGui::End();

		ImGui::Begin("Other");

			const char* guizmoMode = "None";
			switch (m_GuizmoType)
			{
			case -1:                                guizmoMode = "None";      break;
			case ImGuizmo::OPERATION::TRANSLATE:    guizmoMode = "Translate"; break;
			case ImGuizmo::OPERATION::ROTATE:       guizmoMode = "Rotate";    break;
			case ImGuizmo::OPERATION::SCALE:        guizmoMode = "Scale";     break;
			}
			ImGui::Text("Current Guizmo mode: %s", guizmoMode);
		
			ImGui::Separator();

			std::string name = "None";
			if (m_HoveredEntity)
				name = m_HoveredEntity.GetComponent<TagComponent>().Tag;

			ImGui::Text("Hovered Entity: %s", name.c_str());

			ImGui::Separator();

			if (!m_Profiling && ImGui::Button("Start Profiling")) {
				m_Profiling = true;
				RD_PROFILE_BEGIN_SESSION("Runtime", "RodProfile-Runtime.json");
			}
			if (m_Profiling && ImGui::Button("Stop Profiling")) {
				m_Profiling = false;
				RD_PROFILE_END_SESSION();
			}

		ImGui::End();

		ImGui::Begin("Guide");

			// Guizmos 
			if (ImGui::CollapsingHeader("Guizmos"))
			{
				ImGui::Text("Guizmos Shortcut Keys:");
				ImGui::Separator();
				ImGui::Text("Q = None");
				ImGui::Text("W = Translate");
				ImGui::Text("E = Rotate");
				ImGui::Text("R = Scale");
			}

			// Camera
			if (ImGui::CollapsingHeader("Editor Camera"))
			{
				ImGui::Text("Camera Controls:");
				ImGui::Separator();
				ImGui::Text("Alt + Left Mouse = Rotate");
				ImGui::Text("Alt + Middle Mouse = Translate");
				ImGui::Text("Alt + Right Mouse = Zoom");
				ImGui::Text("Mouse Scroll = Zoom");
			}

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");

		auto viewportOffset = ImGui::GetCursorPos();

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->SetBlockEvents(!m_ViewportFocused && !m_ViewportHovered);

		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		ImVec2 viewPortPanelSize = ImGui::GetContentRegionAvail();
		m_PendingViewportSize = { viewPortPanelSize.x, viewPortPanelSize.y };

		ImGui::Image((uint64_t)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, { 0.0f, 1.0f }, { 1.0f, 0.0f });

		ImVec2 minBound = ImGui::GetWindowPos();
		minBound.x += viewportOffset.x;
		minBound.y += viewportOffset.y;
		ImVec2 maxBound = { minBound.x + m_ViewportSize.x, minBound.y + m_ViewportSize.y };
		m_ViewportBounds[0] = { minBound.x, minBound.y };
		m_ViewportBounds[1] = { maxBound.x, maxBound.y };

		// Gizmos
		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selectedEntity && m_GuizmoType != -1)
		{
			// Runtime camera
			//auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
			//const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
			//const glm::mat4& cameraProjection = camera.GetProjection();
			//glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());

			// Editor camera 
			const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
			glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();


			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();

			bool snap = Input::IsKeyPressed(Key::LeftControl);
			float snapValue = 0.5f;
			if (m_GuizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), (ImGuizmo::OPERATION)m_GuizmoType, ImGuizmo::MODE::LOCAL, glm::value_ptr(transform), nullptr, snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				glm::vec3 translation, rotation, scale;
				Math::DecomposeTransform(transform, translation, rotation, scale);

				tc.Translation = translation;
				tc.Rotation = rotation;
				tc.Scale = scale;
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& event)
	{
		m_CameraController.OnEvent(event);
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

}