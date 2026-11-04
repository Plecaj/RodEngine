#include "rdpch.h"

#include "PerformancePanel.h"

#include "Rod/Renderer/Renderer2D.h"

#include "imgui.h"

namespace Rod {

	void PerformancePanel::OnImGuiRender(float deltaTime)
	{
		ImGui::Begin("Performance");

		constexpr float millisecondsPerSecond = 1000.0f;
		const float fps = deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f;
		ImGui::TextDisabled("Frame");
		ImGui::Text("FPS: %.1f", fps);
		ImGui::Text("Frame Time: %.3f ms", deltaTime * millisecondsPerSecond);

		ImGui::Separator();

		auto stats = Renderer2D::GetStats();
		ImGui::TextDisabled("Renderer 2D");
		ImGui::Text("Draw Calls: %u", stats.DrawCalls);
		ImGui::Text("Quad Count: %u", stats.QuadCount);

		ImGui::End();
	}

}
