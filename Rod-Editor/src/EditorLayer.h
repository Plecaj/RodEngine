#pragma once

#include "Rod.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/TitlebarPanel.h"

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

		void NewScene();
		void OpenScene();
		void SaveScene();
		void SaveSceneAs();
	private:
		Ref<Scene> m_ActiveScene;
		Entity m_CameraEntity, m_SecondCameraEntity;

		bool m_PrimaryCamera = true;

		OrthographicCameraController m_CameraController;
		Ref<FrameBuffer> m_Framebuffer;

		bool m_ViewportFocused, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize, m_PendingViewportSize;

		bool m_Profiling = false;

		std::string m_SceneOutputFilepath = std::string();

		// Panels
		TitlebarPanel m_TitlebarPanel;
		friend class TitlebarPanel;
		SceneHierarchyPanel	m_SceneHierarchyPanel;
	};

}