#include "ToolbarPanel.h"
#include "imgui.h"

namespace Rod {

	void ToolbarPanel::OnImGuiRender(SceneState sceneState, Ref<Texture2D> playButton, Ref<Texture2D> stopButton,
		std::function<void()> onPlay, std::function<void()> onStop)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		float size = ImGui::GetWindowHeight() - 4.0f;
		Ref<Texture2D> icon = sceneState == SceneState::Edit ? playButton : stopButton;
		ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x * 0.5f - (size * 0.5f));
		if (ImGui::ImageButton("play/stop button", icon->GetRendererID(), { size, size }, { 0.0f, 1.0f }, { 1.0f, 0.0f }))
		{
			if (sceneState == SceneState::Edit)
				onPlay();
			else if (sceneState == SceneState::Play)
				onStop();
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		ImGui::End();
	}

}
