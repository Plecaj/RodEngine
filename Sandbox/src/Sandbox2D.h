#pragma once

#include "Rod.h"

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
	
	Rod::Ref<Rod::Texture2D> m_Texture;

	struct ProfileResult
	{
		const char* Name;
		float Time;
	};
	std::vector<ProfileResult> m_ProfileResuilts;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
};