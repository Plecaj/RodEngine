#include "rdpch.h"

#include "Rod/Core/Application.h"
#include "Rod/Core/EntryPoint.h" // IWYU pragma: keep
#include "Rod/Core/Layer.h"

#include "imgui.h"

#include <array>

namespace Rod {

	namespace {
		constexpr auto g_WindowTitle = "RodEngine - Project Browser";
		constexpr uint32_t g_LauncherWidth = 768;
		constexpr uint32_t g_LauncherHeight = 396;
		constexpr float g_TitlebarHeight = 42.0f;
		constexpr float g_WindowButtonWidth = 40.0f;
		constexpr float g_ButtonCount = 2.0f;
		constexpr ImVec2 g_TitleTextOffset{16.0f, 12.0f};
		constexpr ImVec2 g_ActionCardSize{104.0f, 104.0f};

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
			DrawProjectActions();

			ImGui::End();
			constexpr int styleVarCount = 2;
			ImGui::PopStyleVar(styleVarCount);
		}

	private:
		enum class ProjectAction : uint8_t
		{
			New,
			Open
		};

		static void DrawProjectActions()
		{
			constexpr float half = 0.5f;
			constexpr float centerOffset = 0.185f;
			constexpr float cardTop = 78.0f;
			constexpr float dividerY = 214.0f;
			constexpr float sidePadding = 16.0f;
			const float width = ImGui::GetWindowWidth();

			ImGui::SetCursorPos(ImVec2{width * (half - centerOffset) - g_ActionCardSize.x * half, cardTop});
			DrawActionCard("New", ProjectAction::New);
			ImGui::SetCursorPos(ImVec2{width * (half + centerOffset) - g_ActionCardSize.x * half, cardTop});
			DrawActionCard("Open", ProjectAction::Open);

			const ImVec2 origin = ImGui::GetWindowPos();
			ImGui::GetWindowDrawList()->AddLine(
			    ImVec2{origin.x + sidePadding, origin.y + dividerY},
			    ImVec2{origin.x + width - sidePadding, origin.y + dividerY},
			    ImGui::GetColorU32(ImGuiCol_Separator));
		}

		static void DrawActionCard(const char* label, ProjectAction action)
		{
			constexpr float half = 0.5f;
			constexpr float iconTop = 38.0f;
			constexpr float labelTop = 66.0f;
			ImGui::PushID(label);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
			ImGui::Button("##Action", g_ActionCardSize);
			ImGui::PopStyleVar();

			const ImVec2 min = ImGui::GetItemRectMin();
			const float centerX = min.x + g_ActionCardSize.x * half;
			const float labelWidth = ImGui::CalcTextSize(label).x;
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			const ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
			DrawActionIcon(*drawList, action, ImVec2{centerX, min.y + iconTop}, color);
			drawList->AddText(ImVec2{centerX - labelWidth * half, min.y + labelTop}, color, label);
			ImGui::PopID();
		}

		static void DrawActionIcon(ImDrawList& drawList, ProjectAction action, const ImVec2& center, ImU32 color)
		{
			constexpr float stroke = 2.0f;
			if (action == ProjectAction::New)
			{
				constexpr float radius = 9.0f;
				drawList
				    .AddLine(ImVec2{center.x - radius, center.y}, ImVec2{center.x + radius, center.y}, color, stroke);
				drawList
				    .AddLine(ImVec2{center.x, center.y - radius}, ImVec2{center.x, center.y + radius}, color, stroke);
				return;
			}

			constexpr auto folderOutline = std::to_array<ImVec2>(
			    {{-13.0f, 10.0f},
			     {-13.0f, -7.0f},
			     {-10.0f, -10.0f},
			     {-5.0f, -10.0f},
			     {-1.0f, -6.0f},
			     {10.0f, -6.0f},
			     {10.0f, 0.0f},
			     {14.0f, 0.0f},
			     {10.0f, 10.0f}});
			auto points = folderOutline;
			for (ImVec2& point : points)
			{
				point.x += center.x;
				point.y += center.y;
			}
			drawList.AddPolyline(points.data(), static_cast<int>(points.size()), color, ImDrawFlags_Closed, stroke);
		}

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
