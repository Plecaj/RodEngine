#pragma once

#include "Rod.h"

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
		Ref<Scene> m_ActiveScene;
		entt::entity m_Square;

		OrthographicCameraController m_CameraController;
		Ref<FrameBuffer> m_Framebuffer;

		Ref<Texture2D> m_Texture;

		bool m_ViewportFocused, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize;

		bool m_Profiling = false;
	};

}