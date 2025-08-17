#include "rdpch.h"
#include "TitlebarPanel.h"

#include "Rod/Core/Application.h"

#include "../EditorLayer.h"

#include <glm/glm.hpp>

namespace Rod{

	TitlebarPanel::TitlebarPanel()
	{
		m_Logo = Texture2D::Create("assets/textures/titlebarLogo.png");
		m_ButtonSize = ImVec2((float)m_Height * 1.25f, (float)m_Height);
	}

	void TitlebarPanel::OnImGuiRender()
	{
		float rowY = ImGui::GetCursorPosY();

		ImVec2 logoSize = { (float)m_Height, (float)m_Height };
		ImGui::Image(m_Logo->GetRendererID(), logoSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

		ImGui::SameLine();
		ImGui::TextUnformatted("Rod-Editor");

		ImGui::SameLine();
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
		if (ImGui::Button("File"))
			ImGui::OpenPopup("FileMenuPopup");

		if (ImGui::BeginPopup("FileMenuPopup"))
		{
			if (ImGui::MenuItem(" New", "Ctrl+N"))           
				m_NewScene();
			if (ImGui::MenuItem(" Open...", "Ctrl+O"))       
				m_OpenScene();
			if (ImGui::MenuItem(" Save", "Ctrl+S"))          
				m_SaveScene();
			if (ImGui::MenuItem(" Save as...", "Ctrl+Shift+S")) 
				m_SaveSceneAs();
			if (ImGui::MenuItem(" Exit"))                    
				Application::Get().Close();
			ImGui::EndPopup();
		}
	}

	void TitlebarPanel::DrawButtons()
	{
		if (ImGui::Button("_", m_ButtonSize))
			Application::Get().Minimalize();

		ImGui::SameLine(0.0f, 0.0f);
		if (ImGui::Button("[]", m_ButtonSize)) 
		{ 
			if (Application::Get().IsMaximized()) 
				Application::Get().RestoreWindow(); 
			else 
				Application::Get().Maximalize(); 
		}

		ImGui::SameLine(0.0f, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.1f, 0.1f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));

		if (ImGui::Button("X", m_ButtonSize))
			Application::Get().Close();

		ImGui::PopStyleColor(3);
	}

}