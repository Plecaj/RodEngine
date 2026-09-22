#include "PerformancePanel.h"
#include "imgui.h"
#include "Rod/Renderer/Renderer2D.h"

namespace Rod {

	void PerformancePanel::OnImGuiRender(float deltaTime)
	{
		ImGui::Begin("Performance");

		float fps = deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f;
		ImGui::TextDisabled("Frame");
		ImGui::Text("FPS: %.1f", fps);
		ImGui::Text("Frame Time: %.3f ms", deltaTime * 1000.0f);

		ImGui::Separator();

		auto stats = Renderer2D::GetStats();
		ImGui::TextDisabled("Renderer 2D");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quad Count: %d", stats.QuadCount);

		ImGui::End();
	}

}
