#pragma once

#include "Rod.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/TitlebarPanel.h"

#include "Rod/Renderer/EditorCamera.h"

namespace Rod {

	class EditorLayer : public Rod::Layer {
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		void OnEvent(Event& event) override;
	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveScene();
		void SaveSceneAs();

		void OnScenePlay();
		void OnSceneStop();

		void UI_Toolbar();
	private:
		Ref<Scene> m_ActiveScene;
		Entity m_CameraEntity;

		OrthographicCameraController m_CameraController;

		EditorCamera m_EditorCamera;

		Ref<Framebuffer> m_Framebuffer;

		bool m_ViewportFocused, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize, m_PendingViewportSize;
		glm::vec2 m_ViewportBounds[2];

		bool m_Profiling = false;

		std::string m_SceneOutputFilepath = std::string();

		Entity m_HoveredEntity;

		int m_GuizmoType = -1;

		float m_LastDeltaTime = 0.0f;

		enum class SceneState
		{
			Edit = 0, Play = 1
		};

		SceneState m_SceneState = SceneState::Edit;

		// Panels
		TitlebarPanel m_TitlebarPanel;
		friend class TitlebarPanel;
		SceneHierarchyPanel	m_SceneHierarchyPanel;
		ContentBrowserPanel	m_ContentBrowserPanel;


		// Editor Resources
		Ref<Texture2D> m_PlayButton, m_StopButton;
	};

}