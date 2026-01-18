#include "DebugPanel.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "Rod/Debug/Instrumentator.h"

namespace Rod {

	void DebugPanel::OnImGuiRender(int guizmoType, Entity hoveredEntity, bool& profiling)
	{
		ImGui::Begin("Other");

		const char* guizmoMode = "None";
		switch (guizmoType)
		{
		case -1:                                guizmoMode = "None";      break;
		case ImGuizmo::OPERATION::TRANSLATE:    guizmoMode = "Translate"; break;
		case ImGuizmo::OPERATION::ROTATE:       guizmoMode = "Rotate";    break;
		case ImGuizmo::OPERATION::SCALE:        guizmoMode = "Scale";     break;
		}
		ImGui::Text("Current Guizmo mode: %s", guizmoMode);

		ImGui::Separator();

		std::string name = "None";
		if (hoveredEntity)
			name = hoveredEntity.GetComponent<TagComponent>().Tag;

		ImGui::Text("Hovered Entity: %s", name.c_str());

		ImGui::Separator();

		if (!profiling && ImGui::Button("Start Profiling")) {
			profiling = true;
			RD_PROFILE_BEGIN_SESSION("Runtime", "RodProfile-Runtime.json");
		}
		if (profiling && ImGui::Button("Stop Profiling")) {
			profiling = false;
			RD_PROFILE_END_SESSION();
		}

		ImGui::End();
	}

}
