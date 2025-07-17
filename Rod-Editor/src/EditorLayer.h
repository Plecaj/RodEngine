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
		OrthographicCameraController m_CameraController;
		Ref<FrameBuffer> m_Framebuffer;

		Ref<Texture2D> m_Texture;

		Ref<Texture2D> m_SpriteSheet;
		Ref<SubTexture2D> m_TextureBarrel;
		uint32_t m_MapWidth, m_MapHeight;
		std::unordered_map<char, Ref<SubTexture2D>> m_TextureMap;

		glm::vec2 m_ViewportSize;

		bool m_Profiling = false;
	};

}