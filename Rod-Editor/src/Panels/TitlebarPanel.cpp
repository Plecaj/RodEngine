#include "rdpch.h"
#include "TitlebarPanel.h"

#include "Rod/Core/Application.h"

#include "../EditorLayer.h"

#include <glm/glm.hpp>

namespace Rod{

	TitlebarPanel::TitlebarPanel()
		: m_ButtonSize({40.0f, 24.0f})
	{
	}

	void TitlebarPanel::OnImGuiRender()
	{
		ImGui::Text("Rod-Editor");

		DrawFileMenu();
		DrawButtons();
		HandleDrag();   
	}

	void TitlebarPanel::DrawFileMenu()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New", "Ctrl+N"))
				m_NewScene();

			if (ImGui::MenuItem("Open...", "Ctrl+O"))
				m_OpenScene();

			if (ImGui::MenuItem("Save", "Ctrl+S"))
				m_SaveScene();

			if (ImGui::MenuItem("Save as...", "Ctrl+Shift+S"))
				m_SaveSceneAs();

			if (ImGui::MenuItem("Exit")) { Application::Get().Close(); }
			ImGui::EndMenu();
		}
	}

	void TitlebarPanel::HandleDrag()
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();

		float buttonAreaWidth = m_ButtonSize.x * 3;

		ImVec2 titlebarSize = { viewport->WorkSize.x - buttonAreaWidth, (float)m_Height };

		ImGui::SetCursorPos(ImVec2(0, 0));
		ImGui::InvisibleButton("TitlebarDragZone", titlebarSize, ImGuiButtonFlags_MouseButtonLeft);

		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			Application::Get().BeginWindowDrag();
		}
	}

	void TitlebarPanel::DrawButtons()
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImVec2 buttonAreaStart = ImVec2(viewport->WorkSize.x - m_ButtonSize.x * 3, 0);
		ImGui::SetCursorScreenPos(ImVec2(viewport->Pos.x + buttonAreaStart.x, viewport->Pos.y));

		if (ImGui::Button("_", m_ButtonSize))
		{
			Application::Get().Minimalize();
		}
		ImGui::SameLine(0.0f, 0.0f);

		if (ImGui::Button("[]", m_ButtonSize))
		{
			if (Application::Get().IsMaximalized())
				Application::Get().RestoreWindow();
			else
				Application::Get().Maximalize();
		}
		ImGui::SameLine(0.0f, 0.0f);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.1f, 0.1f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));

		if (ImGui::Button("X", m_ButtonSize))
		{
			Application::Get().Close();
		}

		ImGui::PopStyleColor(3);
	}

}