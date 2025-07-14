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
	RD_PROFILE_FUNCTION();

	m_Texture = Rod::Texture2D::Create("assets/textures/test.png");
}

void Sandbox2D::OnDetach()
{
	RD_PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(Rod::Timestep ts)
{
	m_CameraController.OnUpdate(ts);

	Rod::Renderer2D::ResetStats();
	Rod::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Rod::RenderCommand::Clear();

	Rod::Renderer2D::BeginScene(m_CameraController.GetCamera());

	Rod::Renderer2D::DrawQuad({ 1.0f, 0.0f }, { 0.5f, 0.5f }, { 0.2f, 0.2f, 0.8f, 1.0f });
	Rod::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.5f, 0.75f }, { 0.8f, 0.2f, 0.3f, 1.0f });
	Rod::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.75f, 0.75f }, m_Texture, 3.0f);
	Rod::Renderer2D::DrawQuad({ 0.0f, 0.0f }, { 0.5f, 0.5f }, m_Texture);
	Rod::Renderer2D::DrawRotatedQuad({ 1.0f, -1.0f }, { 0.25f, 0.25f }, glm::radians(45.0f), { 0.1f, 0.1f, 0.8f, 1.0f });
	Rod::Renderer2D::DrawRotatedQuad({ 0.25f, 0.25f, 1.0f }, { 0.1f, 0.1f }, glm::radians(-45.0f), m_Texture);

	Rod::Renderer2D::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");

	auto stats = Rod::Renderer2D::GetStats();
	ImGui::Text("Renderer Stats 2D:");
	ImGui::Text(	"Draw calls: %d", stats.DrawCalls);
	ImGui::Text(	"Quad Count: %d", stats.QuadCount);

	ImGui::End();
}

void Sandbox2D::OnEvent(Rod::Event& event)
{
	m_CameraController.OnEvent(event);
}
