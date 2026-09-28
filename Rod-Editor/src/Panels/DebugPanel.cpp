#include "DebugPanel.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "Rod/Debug/Instrumentator.h"

namespace Rod {

	void DebugPanel::OnImGuiRender(int guizmoType, Entity hoveredEntity, bool& profiling)
	{
		ImGui::Begin("Other");

		ImGui::TextDisabled("Selection");

		const char* guizmoMode = "None";
		switch (guizmoType)
		{
		case -1:                                guizmoMode = "None";      break;
		case ImGuizmo::OPERATION::TRANSLATE:    guizmoMode = "Translate"; break;
		case ImGuizmo::OPERATION::ROTATE:       guizmoMode = "Rotate";    break;
		case ImGuizmo::OPERATION::SCALE:        guizmoMode = "Scale";     break;
		}
		ImGui::Text("Current Guizmo mode: %s", guizmoMode);

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
