#include "Sandbox2D.h"
#include "imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Rod/Platform/OpenGL/OpenGLShader.h"

#include <chrono>

static const uint32_t s_MapWidth = 24;
static const char* s_MapTiles =
"WWWWWWWWWWWWWWWWWWWWWWWW"
"WWWWWWWWWWDDDDDDWWWWWWWW"
"WWWWDWWWWDDDDDDDDDWWWWWW"
"WWWDDWWDDDWDDDDDDDDDWWWW"
"WWDDDDDDWWWWDDDDDDDDDWWW"
"WDDDDDDWWWWDDDDDDDDDDDDW"
"WWDDDDDDWWDDDDDDDDDDDDWW"
"WWWDDDDDDDDDDDDDDDDDDWWW"
"WWWWDDDDDDDDDDDDDDDDWWWW"
"WWWWWDDDDDDDDDDDDDDWWWWW"
"WWWWWWWWDDDDWWWDWWWWWWWW"
"WWWWWWWWWWWWWWWWWWWWWWWW"
;

Sandbox2D::Sandbox2D()
	:Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f)
{
}

void Sandbox2D::OnAttach()
{
	RD_PROFILE_FUNCTION();

	m_Texture = Rod::Texture2D::Create("assets/textures/test.png");
	m_SpriteSheet = Rod::Texture2D::Create("assets/game/textures/RPGpack_sheet_2X.png");

	m_MapWidth = s_MapWidth;
	m_MapHeight = strlen(s_MapTiles) / m_MapWidth;

	m_TextureMap['W'] = Rod::SubTexture2D::CreateFromCoords(m_SpriteSheet, {11, 11}, {128, 128});
	m_TextureMap['D'] = Rod::SubTexture2D::CreateFromCoords(m_SpriteSheet, {6, 11}, {128, 128});

	m_TextureBarrel = Rod::SubTexture2D::CreateFromCoords(m_SpriteSheet, {9, 1}, { 128, 128 });

	m_Particle.ColorBegin = { 254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f };
	m_Particle.ColorEnd = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f };
	m_Particle.SizeBegin = 0.5f, m_Particle.SizeVariation = 0.3f, m_Particle.SizeEnd = 0.0f;
	m_Particle.LifeTime = 2.0f;
	m_Particle.Velocity = { 0.0f, 0.0f };
	m_Particle.VelocityVariation = { 3.0f, 1.0f };
	m_Particle.Position = { 0.0f, 0.0f };

	m_CameraController.SetZoomLevel(7.0f);
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

#if 0
	Rod::Renderer2D::BeginScene(m_CameraController.GetCamera());

	Rod::Renderer2D::DrawQuad({ 1.0f, 0.0f }, { 0.5f, 0.5f }, { 0.2f, 0.2f, 0.8f, 1.0f });
	Rod::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.5f, 0.75f }, { 0.8f, 0.2f, 0.3f, 1.0f });
	Rod::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.75f, 0.75f }, m_Texture, 3.0f);
	Rod::Renderer2D::DrawQuad({ 0.0f, 0.0f }, { 0.5f, 0.5f }, m_Texture);
	Rod::Renderer2D::DrawRotatedQuad({ 1.0f, -1.0f }, { 0.25f, 0.25f }, glm::radians(45.0f), { 0.1f, 0.1f, 0.8f, 1.0f });
	Rod::Renderer2D::DrawRotatedQuad({ 0.25f, 0.25f, 1.0f }, { 0.1f, 0.1f }, glm::radians(-45.0f), m_Texture);

	Rod::Renderer2D::EndScene();
#endif
	if (Rod::Input::IsMouseButtonPressed(Rod::Mouse::Button0))
	{
		auto [x, y] = Rod::Input::GetMousePosition();
		auto width = Rod::Application::Get().GetWindow().GetWidth();
		auto height = Rod::Application::Get().GetWindow().GetHeight();

		auto bounds = m_CameraController.GetBounds();
		auto pos = m_CameraController.GetCamera().GetPosition();
		x = (x / width) * bounds.GetWidth() - bounds.GetWidth() * 0.5f;
		y = bounds.GetHeight() * 0.5f - (y / height) * bounds.GetHeight();
		m_Particle.Position = { x + pos.x, y + pos.y };
		for (int i = 0; i < 5; i++)
			m_ParticleSystem.Emit(m_Particle);
	}

	m_ParticleSystem.OnUpdate(ts);
	m_ParticleSystem.OnRender(m_CameraController.GetCamera());

	Rod::Renderer2D::BeginScene(m_CameraController.GetCamera());

	for (uint32_t y = 0; y < m_MapHeight; y++) 
	{
		for (uint32_t x = 0; x < m_MapWidth; x++)
		{
			char tileType = s_MapTiles[x + y * m_MapWidth];
			Rod::Ref<Rod::SubTexture2D> texture;
			if (m_TextureMap.find(tileType) != m_TextureMap.end()) {
				texture = m_TextureMap[tileType];
			}
			else {
				texture = m_TextureBarrel;
			}

			Rod::Renderer2D::DrawQuad({ x - m_MapWidth / 2.0f, y - m_MapHeight / 2.0f }, { 1.0f, 1.0f }, texture);
		}
	}

	Rod::Renderer2D::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
	RD_PROFILE_FUNCTION();

	ImGui::Begin("Settings");

	auto stats = Rod::Renderer2D::GetStats();
	ImGui::Text("Renderer Stats 2D:");
	ImGui::Text("	Draw calls: %d", stats.DrawCalls);
	ImGui::Text("	Quad Count: %d", stats.QuadCount);

    ImGui::End();
}

void Sandbox2D::OnEvent(Rod::Event& event)
{
	m_CameraController.OnEvent(event);
}
