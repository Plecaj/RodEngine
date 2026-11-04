#include "rdpch.h"

#include "ToolbarPanel.h"

#include "imgui.h"

namespace Rod {

	void ToolbarPanel::OnImGuiRender(
	    SceneState sceneState,
	    const Texture2D& playButton,
	    const Texture2D& stopButton,
	    ImGuiID dockID,
	    const std::function<void()>& onPlay,
	    const std::function<void()>& onStop)
	{
		constexpr ImVec2 windowPadding{0, 2};
		constexpr float buttonAlpha = 0.5f;
		constexpr float buttonInset = 4.0f;
		constexpr float centerFactor = 0.5f;
		constexpr int styleVariableCount = 2;
		constexpr int styleColorCount = 3;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, windowPadding);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(
		    ImGuiCol_ButtonHovered,
		    ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, buttonAlpha));
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(
		    ImGuiCol_ButtonActive,
		    ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, buttonAlpha));

		const ImGuiWindowFlags toolbarFlags =
		    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

		if (dockID != 0)
			ImGui::SetNextWindowDockID(dockID, ImGuiCond_Always);
		ImGui::Begin("##toolbar", nullptr, toolbarFlags);

		const float size = ImGui::GetWindowHeight() - buttonInset;
		const Texture2D& icon = sceneState == SceneState::Edit ? playButton : stopButton;
		ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x * centerFactor - (size * centerFactor));
		if (ImGui::ImageButton("play/stop button", icon.GetRendererID(), {size, size}, {0.0f, 1.0f}, {1.0f, 0.0f}))
		{
			if (sceneState == SceneState::Edit)
				onPlay();
			else if (sceneState == SceneState::Play)
				onStop();
		}

		ImGui::PopStyleVar(styleVariableCount);
		ImGui::PopStyleColor(styleColorCount);
		ImGui::End();
	}

}
