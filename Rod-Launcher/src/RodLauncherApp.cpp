#include "rdpch.h"

#include "Rod/Core/Application.h"
#include "Rod/Core/EntryPoint.h" // IWYU pragma: keep
#include "Rod/Core/Layer.h"
#include "Rod/Renderer/Texture.h"

#include "imgui.h"

#include <array>

namespace Rod {

	namespace {
		constexpr auto g_WindowTitle = "RodEngine - Project Browser";
		constexpr uint32_t g_LauncherWidth = 700;
		constexpr uint32_t g_LauncherHeight = 440;
		constexpr ImVec2 g_ActionCardSize{104.0f, 104.0f};
		constexpr float g_ContentPadding = 16.0f;
		constexpr float g_IconStroke = 2.0f;
		constexpr float g_RecentRowHeight = 52.0f;

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

		void OnAttach() override { m_FolderIcon = Texture2D::Create("assets/textures/DirectoryIcon.png"); }

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
			DrawRecentProjects();

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

		struct RecentProject
		{
			const char* Name;
			const char* Path;
			const char* LastOpened;
		};

		void DrawRecentProjects() const
		{
			constexpr auto projects = std::to_array<RecentProject>({
			    {"Sandbox", R"(C:\Dev\RodEngine\Sandbox)", "Yesterday"},
			    {"LightingTests", R"(D:\Projects\LightingTests)", "Nov 4"},
			    {"TestScene", R"(C:\Projects\TestScene)", "Oct 28"},
			});
			constexpr float headingTop = 220.0f;
			constexpr float rowsTop = 248.0f;
			constexpr float visibleRowCount = 3.0f;
			const float width = ImGui::GetWindowWidth() - g_ContentPadding - g_ContentPadding;
			ImGui::SetCursorPos(ImVec2{g_ContentPadding, headingTop});
			ImGui::TextUnformatted("Recent Projects");
			ImGui::SetCursorPos(ImVec2{g_ContentPadding, rowsTop});
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0.0f, 0.0f});
			if (ImGui::BeginChild("RecentProjects", ImVec2{width, visibleRowCount * g_RecentRowHeight}))
			{
				for (const RecentProject& project : projects)
					DrawRecentProject(project);
			}
			ImGui::EndChild();
			ImGui::PopStyleVar();
		}

		void DrawRecentProject(const RecentProject& project) const
		{
			constexpr ImVec2 nameOffset{28.0f, 6.0f};
			constexpr ImVec2 iconOffset{8.0f, 26.0f};
			constexpr float pathTop = 29.0f;
			constexpr float dateTop = 19.0f;
			constexpr float detailFontSize = 13.0f;
			constexpr float folderScale = 0.5f;
			const float width = ImGui::GetContentRegionAvail().x;
			ImGui::PushID(project.Path);
			ImGui::Selectable("##Project", false, ImGuiSelectableFlags_None, ImVec2{width, g_RecentRowHeight});
			const ImVec2 origin = ImGui::GetItemRectMin();
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			const ImU32 mutedColor = ImGui::GetColorU32(ImGuiCol_TextDisabled);
			const float dateWidth = ImGui::CalcTextSize(project.LastOpened).x * detailFontSize / ImGui::GetFontSize();
			DrawFolderIcon(
			    *drawList,
			    ImVec2{origin.x + iconOffset.x, origin.y + iconOffset.y},
			    mutedColor,
			    folderScale);
			drawList->AddText(
			    ImVec2{origin.x + nameOffset.x, origin.y + nameOffset.y},
			    ImGui::GetColorU32(ImGuiCol_Text),
			    project.Name);
			drawList->AddText(
			    ImGui::GetFont(),
			    detailFontSize,
			    ImVec2{origin.x + nameOffset.x, origin.y + pathTop},
			    mutedColor,
			    project.Path);
			drawList->AddText(
			    ImGui::GetFont(),
			    detailFontSize,
			    ImVec2{origin.x + width - dateWidth, origin.y + dateTop},
			    mutedColor,
			    project.LastOpened);
			ImGui::PopID();
		}

		void DrawProjectActions() const
		{
			constexpr float half = 0.5f;
			constexpr float centerOffset = 0.185f;
			constexpr float cardTop = 48.0f;
			constexpr float dividerY = 196.0f;
			const float width = ImGui::GetWindowWidth();

			ImGui::SetCursorPos(ImVec2{width * (half - centerOffset) - g_ActionCardSize.x * half, cardTop});
			DrawActionCard("New", ProjectAction::New);
			ImGui::SetCursorPos(ImVec2{width * (half + centerOffset) - g_ActionCardSize.x * half, cardTop});
			DrawActionCard("Open", ProjectAction::Open);

			const ImVec2 origin = ImGui::GetWindowPos();
			ImGui::GetWindowDrawList()->AddLine(
			    ImVec2{origin.x + g_ContentPadding, origin.y + dividerY},
			    ImVec2{origin.x + width - g_ContentPadding, origin.y + dividerY},
			    ImGui::GetColorU32(ImGuiCol_Separator));
		}

		void DrawActionCard(const char* label, ProjectAction action) const
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

		void DrawActionIcon(ImDrawList& drawList, ProjectAction action, const ImVec2& center, ImU32 color) const
		{
			if (action == ProjectAction::New)
			{
				constexpr float radius = 9.0f;
				drawList.AddLine(
				    ImVec2{center.x - radius, center.y},
				    ImVec2{center.x + radius, center.y},
				    color,
				    g_IconStroke);
				drawList.AddLine(
				    ImVec2{center.x, center.y - radius},
				    ImVec2{center.x, center.y + radius},
				    color,
				    g_IconStroke);
				return;
			}
			DrawFolderIcon(drawList, center, color, 1.0f);
		}

		void DrawFolderIcon(ImDrawList& drawList, const ImVec2& center, ImU32 color, float scale) const
		{
			constexpr float halfSize = 16.0f;
			const float radius = halfSize * scale;
			drawList.AddImage(
			    static_cast<ImTextureID>(m_FolderIcon->GetRendererID()),
			    ImVec2{center.x - radius, center.y - radius},
			    ImVec2{center.x + radius, center.y + radius},
			    ImVec2{0.0f, 1.0f},
			    ImVec2{1.0f, 0.0f},
			    color);
		}

	private:
		Scope<Texture2D> m_FolderIcon;
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
