#include "Sandbox2D.h"
#include "imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Rod/Platform/OpenGL/OpenGLShader.h"

#include <chrono>

template<typename Fn>
class Timer {
public:
	Timer(const char* name, Fn&& func)
		:m_Name(name), m_Stopped(false), m_Func(func) 
	{
		m_StartTimepoint = std::chrono::high_resolution_clock::now();
	}

	~Timer(){if (!m_Stopped) Stop();}

	void Stop() 
	{
		auto endTimepoint = std::chrono::high_resolution_clock::now();

		long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
		long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

		m_Stopped = true;
		float duration = (end - start) * 0.001f;
		m_Func({ m_Name, duration });
	}
private:
	const char* m_Name;
	Fn m_Func;
	std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
	bool m_Stopped;
};

#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, [&](ProfileResult profileResult) {m_ProfileResuilts.push_back(profileResult); });

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
	PROFILE_SCOPE("Sandbox2D::OnUpdate")
	{
		PROFILE_SCOPE("CameraController::OnUpdate");
		m_CameraController.OnUpdate(ts);
	}

	{
		PROFILE_SCOPE("Render prep");
		Rod::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Rod::RenderCommand::Clear();
	}

	{
		PROFILE_SCOPE("Render draw");
		Rod::Renderer2D::BeginScene(m_CameraController.GetCamera());

		Rod::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.5f, 0.5f }, { 0.8f, 0.2f, 0.3f, 1.0f }, 30.0f);
		Rod::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.75f, 0.75f }, m_Texture);

		Rod::Renderer::EndScene();
	}
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");

	for (const auto& result : m_ProfileResuilts) 
	{
		char label[100];
		strcpy(label, "%.3fms  ");
		strcat(label, result.Name);
		ImGui::Text(label, result.Time);
	}
	m_ProfileResuilts.clear();

	ImGui::End();
}

void Sandbox2D::OnEvent(Rod::Event& event)
{
	m_CameraController.OnEvent(event);
}
