#include "rdpch.h"

#include "TitlebarPanel.h"

#include "Rod/Core/Application.h"

namespace Rod {

	bool TitlebarPanel::DrawTitlebarButton(const char* id, ButtonIcon icon, const ImVec2& size)
	{
		constexpr float half = 0.5f;
		constexpr ImU32 closeActive = IM_COL32(150, 36, 44, 255);
		constexpr ImU32 closeHovered = IM_COL32(196, 43, 54, 255);
		constexpr ImU32 buttonActive = IM_COL32(66, 72, 80, 255);
		constexpr ImU32 buttonHovered = IM_COL32(50, 56, 63, 255);
		ImGui::InvisibleButton(id, size);

		const bool hovered = ImGui::IsItemHovered();
		const bool active = ImGui::IsItemActive();
		const bool isClose = icon == ButtonIcon::Close;

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const ImVec2 min = ImGui::GetItemRectMin();
		const ImVec2 max = ImGui::GetItemRectMax();
		ImU32 background = IM_COL32(0, 0, 0, 0);

		if (hovered || active)
		{
			if (isClose)
				background = active ? closeActive : closeHovered;
			else
				background = active ? buttonActive : buttonHovered;
		}

		drawList->AddRectFilled(min, max, background, 0.0f);

		const ImVec2 center{(min.x + max.x) * half, (min.y + max.y) * half};
		DrawButtonIcon(*drawList, icon, center);

		return ImGui::IsItemClicked(ImGuiMouseButton_Left);
	}

	void TitlebarPanel::DrawButtonIcon(ImDrawList& drawList, ButtonIcon icon, const ImVec2& center)
	{
		constexpr float half = 0.5f;
		constexpr float restoreOffset = 2.0f;
		constexpr float minimizeOffset = 4.0f;
		constexpr ImU32 iconColor = IM_COL32(230, 233, 238, 255);
		constexpr float stroke = 1.6f;

		switch (icon)
		{
			case ButtonIcon::Minimize: {
				constexpr float width = 10.0f;
				drawList.AddLine(
				    ImVec2(center.x - width * half, center.y + minimizeOffset),
				    ImVec2(center.x + width * half, center.y + minimizeOffset),
				    iconColor,
				    stroke);
				break;
			}
			case ButtonIcon::Maximize: {
				constexpr float side = 10.0f;
				drawList.AddRect(
				    ImVec2(center.x - side * half, center.y - side * half),
				    ImVec2(center.x + side * half, center.y + side * half),
				    iconColor,
				    0.0f,
				    0,
				    stroke);
				break;
			}
			case ButtonIcon::Restore: {
				constexpr float side = 9.0f;
				drawList.AddRect(
				    ImVec2(center.x - side * half - restoreOffset, center.y - side * half + restoreOffset),
				    ImVec2(center.x + side * half - restoreOffset, center.y + side * half + restoreOffset),
				    iconColor,
				    0.0f,
				    0,
				    stroke);
				drawList.AddRect(
				    ImVec2(center.x - side * half + restoreOffset, center.y - side * half - restoreOffset),
				    ImVec2(center.x + side * half + restoreOffset, center.y + side * half - restoreOffset),
				    iconColor,
				    0.0f,
				    0,
				    stroke);
				break;
			}
			case ButtonIcon::Close: {
				constexpr float radius = 5.0f;
				drawList.AddLine(
				    ImVec2(center.x - radius, center.y - radius),
				    ImVec2(center.x + radius, center.y + radius),
				    iconColor,
				    stroke);
				drawList.AddLine(
				    ImVec2(center.x + radius, center.y - radius),
				    ImVec2(center.x - radius, center.y + radius),
				    iconColor,
				    stroke);
				break;
			}
		}
	}

	void TitlebarPanel::RunCallback(const std::function<void()>& callback)
	{
		if (callback)
			callback();
	}

	TitlebarPanel::TitlebarPanel()
	{
		m_Logo = Texture2D::Create("assets/textures/titlebarLogo.png");
		m_ButtonSize = ImVec2(BUTTON_WIDTH, static_cast<float>(HEIGHT));
	}

	void TitlebarPanel::OnImGuiRender()
	{
		constexpr ImU32 backgroundColor = IM_COL32(19, 22, 26, 255);
		constexpr ImU32 borderColor = IM_COL32(45, 51, 60, 255);
		constexpr ImVec2 logoInset{8.0f, 4.0f};
		constexpr float labelTopInset = 8.0f;
		constexpr float menuTopInset = 5.0f;
		constexpr float windowButtonCount = 3.0f;
		const float rowY = ImGui::GetCursorPosY();

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const ImVec2 windowMin = ImGui::GetWindowPos();
		const ImVec2 windowMax =
		    ImVec2(windowMin.x + ImGui::GetWindowWidth(), windowMin.y + static_cast<float>(HEIGHT));
		drawList->AddRectFilled(windowMin, windowMax, backgroundColor);
		drawList->AddLine(
		    ImVec2(windowMin.x, windowMax.y - 1.0f),
		    ImVec2(windowMax.x, windowMax.y - 1.0f),
		    borderColor);

		constexpr ImVec2 logoSize = {28.0f, 28.0f};
		ImGui::SetCursorPos(ImVec2(logoInset.x, rowY + logoInset.y));
		ImGui::Image(
		    static_cast<ImTextureID>(m_Logo->GetRendererID()),
		    logoSize,
		    ImVec2(0.0f, 1.0f),
		    ImVec2(1.0f, 0.0f));

		ImGui::SameLine();
		ImGui::SetCursorPosY(rowY + labelTopInset);
		ImGui::TextUnformatted("Rod-Editor");

		ImGui::SameLine();
		ImGui::SetCursorPosY(rowY + menuTopInset);
		DrawFileMenu();

		const float dragZoneStartX = ImGui::GetCursorPosX();
		const float dragZoneWidth =
		    std::max(ImGui::GetContentRegionAvail().x - (m_ButtonSize.x * windowButtonCount), 0.0f);

		ImGui::SetCursorPos(ImVec2(dragZoneStartX + dragZoneWidth, rowY));
		DrawButtons();

		ImGui::SetCursorPos(ImVec2(dragZoneStartX, rowY));
		ImGui::InvisibleButton("TitlebarDragZone", ImVec2(dragZoneWidth, m_ButtonSize.y));
		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			Application::Get().BeginWindowDrag();
	}

	void TitlebarPanel::DrawFileMenu()
	{
		constexpr ImVec2 framePadding{10.0f, 4.0f};
		constexpr float frameRounding = 3.0f;
		constexpr ImVec2 windowPadding{12.0f, 8.0f};
		constexpr ImVec2 itemSpacing{10.0f, 6.0f};
		constexpr int styleVariableCount = 2;
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, framePadding);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, frameRounding);
		if (ImGui::Button("File"))
			ImGui::OpenPopup("FileMenuPopup");
		ImGui::PopStyleVar(styleVariableCount);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, windowPadding);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, itemSpacing);
		if (ImGui::BeginPopup("FileMenuPopup"))
		{
			if (ImGui::MenuItem("New", "Ctrl+N"))
				RunCallback(m_NewScene);
			if (ImGui::MenuItem("Open...", "Ctrl+O"))
				RunCallback(m_OpenScene);
			if (ImGui::MenuItem("Save", "Ctrl+S"))
				RunCallback(m_SaveScene);
			if (ImGui::MenuItem("Save as...", "Ctrl+Shift+S"))
				RunCallback(m_SaveSceneAs);
			ImGui::Separator();
			if (ImGui::MenuItem("Export game"))
				RunCallback(m_ExportGame);
			ImGui::Separator();
			if (ImGui::MenuItem("Exit"))
				Application::Get().Close();
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar(styleVariableCount);
	}

	void TitlebarPanel::DrawButtons()
	{
		if (DrawTitlebarButton("##Minimize", ButtonIcon::Minimize, m_ButtonSize))
			Application::Get().Minimize();

		ImGui::SameLine(0.0f, 0.0f);
		const ButtonIcon maximizeIcon = Application::Get().IsMaximized() ? ButtonIcon::Restore : ButtonIcon::Maximize;
		if (DrawTitlebarButton("##MaximizeRestore", maximizeIcon, m_ButtonSize))
		{
			if (Application::Get().IsMaximized())
				Application::Get().RestoreWindow();
			else
				Application::Get().Maximize();
		}

		ImGui::SameLine(0.0f, 0.0f);
		if (DrawTitlebarButton("##Close", ButtonIcon::Close, m_ButtonSize))
			Application::Get().Close();
	}

}
