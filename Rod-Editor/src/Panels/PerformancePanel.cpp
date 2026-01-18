#include "PerformancePanel.h"
#include "imgui.h"
#include "Rod/Renderer/Renderer2D.h"

namespace Rod {

	void PerformancePanel::OnImGuiRender(float deltaTime)
	{
		ImGui::Begin("Performance");

		float fps = 1.0f / deltaTime;
		ImGui::Text("FPS: %.1f", fps);
		ImGui::Text("Frame Time: %.3f ms", deltaTime * 1000.0f);

		ImGui::Separator();

		auto stats = Renderer2D::GetStats();
		ImGui::Text("Renderer Stats:");
		ImGui::Text("	Draw calls: %d", stats.DrawCalls);
		ImGui::Text("	Quad Count: %d", stats.QuadCount);

		ImGui::End();
	}

}
