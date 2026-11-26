#include "rdpch.h"

#include "ToolbarPanel.h"

#include "Rod/ImGui/ImGuiFlags.h"
#include "Rod/Renderer/RenderDevice.h"

#include "imgui.h"

namespace Rod {

	ToolbarPanel::ToolbarPanel(const RenderDevice& device)
	    : m_PlayButton(device.LoadTexture2D("RodAssets/Textures/PlayButton.png")),
	      m_StopButton(device.LoadTexture2D("RodAssets/Textures/StopButton.png"))
	{}

	void ToolbarPanel::OnImGuiRender(SceneState sceneState, ImGuiID dockID, const Actions& actions) const
	{
		constexpr ImVec2 windowPadding{0, 2};
		constexpr float buttonAlpha = 0.5f;
		constexpr float buttonInset = 4.0f;
		constexpr int styleVariableCount = 2;
		constexpr int styleColorCount = 3;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, windowPadding);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		const auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(
		    ImGuiCol_ButtonHovered,
		    ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, buttonAlpha));
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(
		    ImGuiCol_ButtonActive,
		    ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, buttonAlpha));

		const ImGuiWindowFlags toolbarFlags = CombineImGuiFlags(
		    ImGuiWindowFlags_NoDecoration,
		    ImGuiWindowFlags_NoScrollbar,
		    ImGuiWindowFlags_NoScrollWithMouse);

		if (dockID != 0)
			ImGui::SetNextWindowDockID(dockID, ImGuiCond_Always);
		ImGui::Begin("##toolbar", nullptr, toolbarFlags);

		const float size = ImGui::GetWindowHeight() - buttonInset;
		const Texture2D& icon = sceneState == SceneState::Edit ? *m_PlayButton : *m_StopButton;
		ImGui::SetCursorPosX((ImGui::GetContentRegionMax().x * 0.5f) - (size * 0.5f));
		if (ImGui::ImageButton("play/stop button", icon.GetRendererID(), {size, size}, {0.0f, 1.0f}, {1.0f, 0.0f}))
		{
			if (sceneState == SceneState::Edit)
			{
				actions.Play();
			}
			else if (sceneState == SceneState::Play)
			{
				actions.Stop();
			}
		}

		ImGui::PopStyleVar(styleVariableCount);
		ImGui::PopStyleColor(styleColorCount);
		ImGui::End();
	}

}
