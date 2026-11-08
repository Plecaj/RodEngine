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
		constexpr uint32_t g_LauncherHeight = 354;
		constexpr ImVec2 g_ActionCardSize{104.0f, 104.0f};

		WindowProps LauncherWindowProps()
		{
			WindowProps props{g_WindowTitle, g_LauncherWidth, g_LauncherHeight, "assets/textures/logo.png", true};
			props.NativeTitlebar = true;
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
			constexpr float cardTop = 36.0f;
			constexpr float dividerY = 172.0f;
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
