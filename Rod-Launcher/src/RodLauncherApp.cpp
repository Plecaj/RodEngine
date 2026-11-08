#include "rdpch.h"

#include "Rod/Core/Application.h"
#include "Rod/Core/EntryPoint.h" // IWYU pragma: keep
#include "Rod/Core/Layer.h"

#include "imgui.h"

namespace Rod {

	namespace {
		constexpr auto g_WindowTitle = "RodEngine - Project Browser";
		constexpr uint32_t g_LauncherWidth = 768;
		constexpr uint32_t g_LauncherHeight = 396;
		constexpr float g_TitlebarHeight = 42.0f;
		constexpr float g_WindowButtonWidth = 40.0f;
		constexpr float g_ButtonCount = 2.0f;
		constexpr ImVec2 g_TitleTextOffset{16.0f, 12.0f};

		WindowProps LauncherWindowProps()
		{
			WindowProps props{g_WindowTitle, g_LauncherWidth, g_LauncherHeight, "assets/textures/logo.png", true};
			props.Resizable = false;
			props.Centered = true;
			return props;
		}
	}

	class LauncherLayer final : public Layer
	{
	public:
		LauncherLayer() : Layer("LauncherLayer") {}

		void OnImGuiRender() override
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0.0f, 0.0f});
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
			                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
			ImGui::Begin("Project Browser", nullptr, flags);

			DrawTitlebar();

			ImGui::End();
			constexpr int styleVarCount = 2;
			ImGui::PopStyleVar(styleVarCount);
		}

	private:
		static void DrawTitlebar()
		{
			const ImVec2 origin = ImGui::GetWindowPos();
			const float width = ImGui::GetWindowWidth();
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			constexpr ImU32 background = IM_COL32(39, 41, 44, 255);
			constexpr ImU32 text = IM_COL32(225, 228, 232, 255);
			drawList->AddRectFilled(origin, ImVec2{origin.x + width, origin.y + g_TitlebarHeight}, background);

			ImGui::SetCursorPos(ImVec2{0.0f, 0.0f});
			ImGui::InvisibleButton(
			    "##DragTitlebar",
			    ImVec2{width - g_WindowButtonWidth * g_ButtonCount, g_TitlebarHeight});
			if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
				Application::Get().BeginWindowDrag();

			drawList->AddText(
			    ImVec2{origin.x + g_TitleTextOffset.x, origin.y + g_TitleTextOffset.y},
			    text,
			    g_WindowTitle);

			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::ColorConvertU32ToFloat4(background));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
			ImGui::SameLine(0.0f, 0.0f);
			if (ImGui::Button("-##Minimize", ImVec2{g_WindowButtonWidth, g_TitlebarHeight}))
				Application::Get().Minimize();
			ImGui::SameLine(0.0f, 0.0f);
			if (ImGui::Button("x##Close", ImVec2{g_WindowButtonWidth, g_TitlebarHeight}))
				Application::Get().Close();
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();
		}
	};

	class RodLauncher final : public Application
	{
	public:
		RodLauncher() : Application(LauncherWindowProps()) { PushLayer(new LauncherLayer()); }
	};

	Application* CreateApplication()
	{
		return new RodLauncher();
	}

}
