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

	m_CameraController.SetZoomLevel(5.0f);
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
	// Dockspace code from ImGui demo
    static bool dockspaceOpen = true;
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
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

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
    if (!opt_padding)
        ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit")) { Rod::Application::Get().Close(); }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

	ImGui::Begin("Settings");

	auto stats = Rod::Renderer2D::GetStats();
	ImGui::Text("Renderer Stats 2D:");
	ImGui::Text("	Draw calls: %d", stats.DrawCalls);
	ImGui::Text("	Quad Count: %d", stats.QuadCount);


	uint32_t textureID = m_Texture->GetRendererID();
	ImVec2 textureSize(128.0f, 128.0f);
	ImGui::Image((uint64_t)textureID, textureSize, {0.0f, 1.0f}, {1.0f, 0.0f});

	ImGui::End();

	Rod::Renderer2D::ResetStats();

    ImGui::End();
}

void Sandbox2D::OnEvent(Rod::Event& event)
{
	m_CameraController.OnEvent(event);
}
