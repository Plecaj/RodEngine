#include "rdpch.h"
#include "TitlebarPanel.h"

#include "Rod/Core/Application.h"

#include "../EditorLayer.h"

#include <glm/glm.hpp>

namespace Rod{

	enum class TitlebarButtonIcon
	{
		Minimize,
		Maximize,
		Restore,
		Close
	};

	static bool DrawTitlebarButton(const char* id, TitlebarButtonIcon icon, const ImVec2& size)
	{
		ImGui::InvisibleButton(id, size);

		bool hovered = ImGui::IsItemHovered();
		bool active = ImGui::IsItemActive();
		bool isClose = icon == TitlebarButtonIcon::Close;

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		ImU32 background = IM_COL32(0, 0, 0, 0);

		if (hovered || active)
		{
			if (isClose)
				background = active ? IM_COL32(150, 36, 44, 255) : IM_COL32(196, 43, 54, 255);
			else
				background = active ? IM_COL32(66, 72, 80, 255) : IM_COL32(50, 56, 63, 255);
		}

		drawList->AddRectFilled(min, max, background, 0.0f);

		ImU32 iconColor = IM_COL32(230, 233, 238, 255);
		ImVec2 center = ImVec2((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f);
		float stroke = 1.6f;

		switch (icon)
		{
			case TitlebarButtonIcon::Minimize:
			{
				float width = 10.0f;
				drawList->AddLine(ImVec2(center.x - width * 0.5f, center.y + 4.0f), ImVec2(center.x + width * 0.5f, center.y + 4.0f), iconColor, stroke);
				break;
			}
			case TitlebarButtonIcon::Maximize:
			{
				float side = 10.0f;
				drawList->AddRect(ImVec2(center.x - side * 0.5f, center.y - side * 0.5f), ImVec2(center.x + side * 0.5f, center.y + side * 0.5f), iconColor, 0.0f, 0, stroke);
				break;
			}
			case TitlebarButtonIcon::Restore:
			{
				float side = 9.0f;
				drawList->AddRect(ImVec2(center.x - side * 0.5f - 2.0f, center.y - side * 0.5f + 2.0f), ImVec2(center.x + side * 0.5f - 2.0f, center.y + side * 0.5f + 2.0f), iconColor, 0.0f, 0, stroke);
				drawList->AddRect(ImVec2(center.x - side * 0.5f + 2.0f, center.y - side * 0.5f - 2.0f), ImVec2(center.x + side * 0.5f + 2.0f, center.y + side * 0.5f - 2.0f), iconColor, 0.0f, 0, stroke);
				break;
			}
			case TitlebarButtonIcon::Close:
			{
				float radius = 5.0f;
				drawList->AddLine(ImVec2(center.x - radius, center.y - radius), ImVec2(center.x + radius, center.y + radius), iconColor, stroke);
				drawList->AddLine(ImVec2(center.x + radius, center.y - radius), ImVec2(center.x - radius, center.y + radius), iconColor, stroke);
				break;
			}
		}

		return ImGui::IsItemClicked(ImGuiMouseButton_Left);
	}

	TitlebarPanel::TitlebarPanel()
	{
		m_Logo = Texture2D::Create("assets/textures/titlebarLogo.png");
		m_ButtonSize = ImVec2(46.0f, (float)m_Height);
	}

	void TitlebarPanel::OnImGuiRender()
	{
		float rowY = ImGui::GetCursorPosY();

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 windowMin = ImGui::GetWindowPos();
		ImVec2 windowMax = ImVec2(windowMin.x + ImGui::GetWindowWidth(), windowMin.y + (float)m_Height);
		drawList->AddRectFilled(windowMin, windowMax, IM_COL32(19, 22, 26, 255));
		drawList->AddLine(ImVec2(windowMin.x, windowMax.y - 1.0f), ImVec2(windowMax.x, windowMax.y - 1.0f), IM_COL32(45, 51, 60, 255));

		ImVec2 logoSize = { 28.0f, 28.0f };
		ImGui::SetCursorPos(ImVec2(8.0f, rowY + 4.0f));
		ImGui::Image((ImTextureID)(uint64_t)m_Logo->GetRendererID(), logoSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

		ImGui::SameLine();
		ImGui::SetCursorPosY(rowY + 8.0f);
		ImGui::TextUnformatted("Rod-Editor");

		ImGui::SameLine();
		ImGui::SetCursorPosY(rowY + 5.0f);
		DrawFileMenu();

		float dragZoneStartX = ImGui::GetCursorPosX();
		float dragZoneWidth = ImGui::GetContentRegionAvail().x - (m_ButtonSize.x * 3.0f);
		if (dragZoneWidth < 0.0f) dragZoneWidth = 0.0f;

		ImGui::SetCursorPos(ImVec2(dragZoneStartX + dragZoneWidth, rowY));
		DrawButtons();

		ImGui::SetCursorPos(ImVec2(dragZoneStartX, rowY));
		ImGui::InvisibleButton("TitlebarDragZone", ImVec2(dragZoneWidth, m_ButtonSize.y));
		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			Application::Get().BeginWindowDrag();
	}

	void TitlebarPanel::DrawFileMenu()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 4.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
		if (ImGui::Button("File"))
			ImGui::OpenPopup("FileMenuPopup");
		ImGui::PopStyleVar(2);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 8.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 6.0f));
		if (ImGui::BeginPopup("FileMenuPopup"))
		{
			if (ImGui::MenuItem("New", "Ctrl+N"))           
				m_NewScene();
			if (ImGui::MenuItem("Open...", "Ctrl+O"))       
				m_OpenScene();
			if (ImGui::MenuItem("Save", "Ctrl+S"))          
				m_SaveScene();
			if (ImGui::MenuItem("Save as...", "Ctrl+Shift+S")) 
				m_SaveSceneAs();
			ImGui::Separator();
			if (ImGui::MenuItem("Exit"))                    
				Application::Get().Close();
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar(2);
	}

	void TitlebarPanel::DrawButtons()
	{
		if (DrawTitlebarButton("##Minimize", TitlebarButtonIcon::Minimize, m_ButtonSize))
			Application::Get().Minimalize();

		ImGui::SameLine(0.0f, 0.0f);
		TitlebarButtonIcon maximizeIcon = Application::Get().IsMaximized() ? TitlebarButtonIcon::Restore : TitlebarButtonIcon::Maximize;
		if (DrawTitlebarButton("##MaximizeRestore", maximizeIcon, m_ButtonSize)) 
		{ 
			if (Application::Get().IsMaximized()) 
				Application::Get().RestoreWindow(); 
			else 
				Application::Get().Maximalize(); 
		}

		ImGui::SameLine(0.0f, 0.0f);
		if (DrawTitlebarButton("##Close", TitlebarButtonIcon::Close, m_ButtonSize))
			Application::Get().Close();
	}

}
