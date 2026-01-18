#include "GuidePanel.h"
#include "imgui.h"

namespace Rod {

	void GuidePanel::OnImGuiRender()
	{
		ImGui::Begin("Guide");

		if (ImGui::CollapsingHeader("Guizmos"))
		{
			ImGui::Text("Guizmos Shortcut Keys:");
			ImGui::Separator();
			ImGui::Text("Q = None");
			ImGui::Text("W = Translate");
			ImGui::Text("E = Rotate");
			ImGui::Text("R = Scale");
		}

		if (ImGui::CollapsingHeader("Editor Camera"))
		{
			ImGui::Text("Camera Controls:");
			ImGui::Separator();
			ImGui::Text("Alt + Left Mouse = Rotate");
			ImGui::Text("Alt + Middle Mouse = Translate");
			ImGui::Text("Alt + Right Mouse = Zoom");
			ImGui::Text("Mouse Scroll = Zoom");
		}

		ImGui::End();
	}

}
