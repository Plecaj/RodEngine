#pragma once

#include "Rod.h"

#include "ParticleSystem.h"

class Sandbox2D : public Rod::Layer{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(Rod::Timestep ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(Rod::Event& event) override;
private:
	Rod::OrthographicCameraController m_CameraController;
	Rod::Ref<Rod::FrameBuffer> m_Framebuffer;
	
	Rod::Ref<Rod::Texture2D> m_Texture;

	ParticleProps m_Particle;
	ParticleSystem m_ParticleSystem;

	Rod::Ref<Rod::Texture2D> m_SpriteSheet;
	Rod::Ref<Rod::SubTexture2D> m_TextureBarrel;
	uint32_t m_MapWidth, m_MapHeight;
	std::unordered_map<char, Rod::Ref<Rod::SubTexture2D>> m_TextureMap;
};