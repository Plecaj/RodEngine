#include "rdpch.h"

#include "DebugPanel.h"

#include "imgui.h"

#include "ImGuizmo.h"

namespace Rod {

	void DebugPanel::OnImGuiRender(int gizmoType, Entity hoveredEntity)
	{
		ImGui::Begin("Other");

		ImGui::TextDisabled("Selection");

		const char* gizmoMode = "None";
		switch (gizmoType)
		{
			case ImGuizmo::OPERATION::TRANSLATE:
				gizmoMode = "Translate";
				break;
			case ImGuizmo::OPERATION::ROTATE:
				gizmoMode = "Rotate";
				break;
			case ImGuizmo::OPERATION::SCALE:
				gizmoMode = "Scale";
				break;
			default:
				break;
		}
		ImGui::Text("Current Gizmo mode: %s", gizmoMode);

		std::string name = "None";
		if (hoveredEntity)
			name = hoveredEntity.GetComponent<TagComponent>().Tag;

		ImGui::Text("Hovered Entity: %s", name.c_str());

		ImGui::End();
	}

}
