#include "DebugPanel.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "Rod/Debug/Instrumentator.h"

namespace Rod {

	void DebugPanel::OnImGuiRender(int gizmoType, Entity hoveredEntity, bool& profiling)
	{
		ImGui::Begin("Other");

		ImGui::TextDisabled("Selection");

		const char* gizmoMode = "None";
		switch (gizmoType)
		{
		case -1:                                gizmoMode = "None";      break;
		case ImGuizmo::OPERATION::TRANSLATE:    gizmoMode = "Translate"; break;
		case ImGuizmo::OPERATION::ROTATE:       gizmoMode = "Rotate";    break;
		case ImGuizmo::OPERATION::SCALE:        gizmoMode = "Scale";     break;
		}
		ImGui::Text("Current Gizmo mode: %s", gizmoMode);

		std::string name = "None";
		if (hoveredEntity)
			name = hoveredEntity.GetComponent<TagComponent>().Tag;

		ImGui::Text("Hovered Entity: %s", name.c_str());

		ImGui::Separator();
		ImGui::TextDisabled("Profiling");

		const char* profilingButtonText = profiling ? "Stop Profiling" : "Start Profiling";
		if (ImGui::Button(profilingButtonText)) {
			if (profiling)
			{
				profiling = false;
				RD_PROFILE_END_SESSION();
				ImGui::End();
				return;
			}

			profiling = true;
			RD_PROFILE_BEGIN_SESSION("Runtime", "RodProfile-Runtime.json");
		}

		ImGui::End();
	}

}
