#include "Sandbox2D.h"
#include "imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Rod/Platform/OpenGL/OpenGLShader.h"

#include <chrono>

Sandbox2D::Sandbox2D()
	:Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f)
{
}

void Sandbox2D::OnAttach()
{
	m_Texture = Rod::Texture2D::Create("assets/textures/test.png");
}

void Sandbox2D::OnDetach()
{
}

void Sandbox2D::OnUpdate(Rod::Timestep ts)
{
	RD_PROFILE_FUNCTION();
	{
		RD_PROFILE_SCOPE("CameraController::OnUpdate");
		m_CameraController.OnUpdate(ts);
	}

	{
		RD_PROFILE_SCOPE("Render prep");
		Rod::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Rod::RenderCommand::Clear();
	}

	{
		RD_PROFILE_SCOPE("Render draw");
		Rod::Renderer2D::BeginScene(m_CameraController.GetCamera());

		Rod::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.5f, 0.5f }, { 0.8f, 0.2f, 0.3f, 1.0f }, 30.0f);
		Rod::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.75f, 0.75f }, m_Texture);

		Rod::Renderer::EndScene();
	}
}

void Sandbox2D::OnImGuiRender()
{
	RD_PROFILE_FUNCTION();
	ImGui::Begin("Settings");

	ImGui::End();
}

void Sandbox2D::OnEvent(Rod::Event& event)
{
	m_CameraController.OnEvent(event);
}
