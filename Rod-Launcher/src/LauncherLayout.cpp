#include "rdpch.h"

#include "LauncherLayout.h"

#include "RecentProjects.h"
#include "Rod/ImGui/ImGuiFlags.h"
#include "Rod/Renderer/RenderDevice.h"

#include <utility>

namespace Rod {

	namespace {
		constexpr ImVec2 g_ActionCardSize{104.0f, 104.0f};
		constexpr float g_ContentPadding = 16.0f;
		constexpr float g_IconStroke = 2.0f;
		constexpr float g_RecentRowHeight = 52.0f;
	}

	LauncherLayout::LauncherLayout(const FileDialogs& dialogs, const RenderDevice& device)
	    : m_ProjectSetup(dialogs), m_FolderIcon(device.LoadTexture2D("RodAssets/Textures/DirectoryIcon.png"))
	{}

	void LauncherLayout::OnImGuiRender(const RecentProjects& recentProjects, const Actions& actions)
	{
		const ImGuiViewport& viewport = *ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport.Pos);
		ImGui::SetNextWindowSize(viewport.Size);
		ImGui::SetNextWindowViewport(viewport.ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0.0f, 0.0f});
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		constexpr ImGuiWindowFlags flags = CombineImGuiFlags(
		    ImGuiWindowFlags_NoDecoration,
		    ImGuiWindowFlags_NoDocking,
		    ImGuiWindowFlags_NoMove,
		    ImGuiWindowFlags_NoSavedSettings);
		ImGui::Begin("Project Browser", nullptr, flags);

		if (m_ProjectSetup.IsOpen())
		{
			if (auto project = m_ProjectSetup.OnImGuiRender())
				ReportProjectError(actions.LaunchEditor(*project));
		}
		else
		{
			if (DrawProjectActions())
				ReportProjectError(actions.OpenProject({}));
			if (const auto selected = DrawRecentProjects(recentProjects))
				ReportProjectError(actions.OpenProject(*selected));
		}
		DrawProjectResult();

		ImGui::End();
		constexpr int styleVarCount = 2;
		ImGui::PopStyleVar(styleVarCount);
	}

	auto LauncherLayout::DrawRecentProjects(const RecentProjects& recentProjects) const
	    -> std::optional<std::filesystem::path>
	{
		constexpr float headingTop = 220.0f;
		constexpr float rowsTop = 248.0f;
		constexpr float visibleRowCount = 3.0f;
		const float width = ImGui::GetWindowWidth() - g_ContentPadding - g_ContentPadding;
		ImGui::SetCursorPos(ImVec2{g_ContentPadding, headingTop});
		ImGui::TextUnformatted("Recent Projects");
		ImGui::SetCursorPos(ImVec2{g_ContentPadding, rowsTop});
		std::optional<std::filesystem::path> selected;
		const auto entries = recentProjects.GetEntries();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0.0f, 0.0f});
		if (ImGui::BeginChild("RecentProjects", ImVec2{width, visibleRowCount * g_RecentRowHeight}))
		{
			if (entries.empty())
				ImGui::TextDisabled("No recent projects. Create or open a project to get started.");
			for (const RecentProject& project : entries)
			{
				if (DrawRecentProject(project))
					selected = project.Path;
			}
		}
		ImGui::EndChild();
		ImGui::PopStyleVar();
		return selected;
	}

	auto LauncherLayout::DrawRecentProject(const RecentProject& project) const -> bool
	{
		constexpr ImVec2 nameOffset{28.0f, 6.0f};
		constexpr ImVec2 iconOffset{8.0f, 26.0f};
		constexpr float pathTop = 29.0f;
		constexpr float dateTop = 6.0f;
		constexpr float dateSpacing = 12.0f;
		constexpr float detailFontSize = 13.0f;
		constexpr float folderScale = 0.5f;
		const float width = ImGui::GetContentRegionAvail().x;
		const auto path = project.Path.string();
		ImGui::PushID(path.c_str());
		const bool clicked =
		    ImGui::Selectable("##Project", false, ImGuiSelectableFlags_None, ImVec2{width, g_RecentRowHeight});
		const ImVec2 origin = ImGui::GetItemRectMin();
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", path.c_str());
		ImDrawList& drawList = *ImGui::GetWindowDrawList();
		const ImU32 mutedColor = ImGui::GetColorU32(ImGuiCol_TextDisabled);
		const float dateWidth =
		    ImGui::CalcTextSize(project.LastOpened.c_str()).x * detailFontSize / ImGui::GetFontSize();
		DrawFolderIcon(drawList, ImVec2{origin.x + iconOffset.x, origin.y + iconOffset.y}, mutedColor, folderScale);
		drawList.PushClipRect(
		    ImVec2{origin.x + nameOffset.x, origin.y},
		    ImVec2{origin.x + width - dateWidth - dateSpacing, origin.y + g_RecentRowHeight},
		    true);
		drawList.AddText(
		    ImVec2{origin.x + nameOffset.x, origin.y + nameOffset.y},
		    ImGui::GetColorU32(ImGuiCol_Text),
		    project.Name.c_str());
		drawList.PopClipRect();
		drawList.AddText(
		    ImGui::GetFont(),
		    detailFontSize,
		    ImVec2{origin.x + nameOffset.x, origin.y + pathTop},
		    mutedColor,
		    path.c_str());
		drawList.AddText(
		    ImGui::GetFont(),
		    detailFontSize,
		    ImVec2{origin.x + width - dateWidth, origin.y + dateTop},
		    mutedColor,
		    project.LastOpened.c_str());
		ImGui::PopID();
		return clicked;
	}

	auto LauncherLayout::DrawProjectActions() -> bool
	{
		constexpr float centerOffset = 0.185f;
		constexpr float cardTop = 48.0f;
		constexpr float dividerY = 196.0f;
		const float width = ImGui::GetWindowWidth();

		ImGui::SetCursorPos(ImVec2{width * (0.5f - centerOffset) - g_ActionCardSize.x * 0.5f, cardTop});
		if (DrawActionCard("New", ProjectAction::New))
			m_ProjectSetup.Open();
		ImGui::SetCursorPos(ImVec2{width * (0.5f + centerOffset) - g_ActionCardSize.x * 0.5f, cardTop});
		const bool open = DrawActionCard("Open", ProjectAction::Open);

		const ImVec2 origin = ImGui::GetWindowPos();
		ImGui::GetWindowDrawList()->AddLine(
		    ImVec2{origin.x + g_ContentPadding, origin.y + dividerY},
		    ImVec2{origin.x + width - g_ContentPadding, origin.y + dividerY},
		    ImGui::GetColorU32(ImGuiCol_Separator));
		return open;
	}

	void LauncherLayout::ReportProjectError(std::optional<std::string> error)
	{
		if (error)
		{
			m_Error = std::move(*error);
			ImGui::OpenPopup("Project");
		}
	}

	void LauncherLayout::DrawProjectResult() const
	{
		constexpr float dialogWidth = 500.0f;
		ImGui::SetNextWindowSize(ImVec2{dialogWidth, 0.0f}, ImGuiCond_Appearing);
		if (ImGui::BeginPopupModal("Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextWrapped("%s", m_Error.c_str());
			if (ImGui::Button("OK"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
	}

	auto LauncherLayout::DrawActionCard(const char* label, ProjectAction action) const -> bool
	{
		constexpr float iconTop = 38.0f;
		constexpr float labelTop = 66.0f;
		ImGui::PushID(label);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
		const bool clicked = ImGui::Button("##Action", g_ActionCardSize);
		ImGui::PopStyleVar();

		const ImVec2 min = ImGui::GetItemRectMin();
		const float centerX = min.x + g_ActionCardSize.x * 0.5f;
		const float labelWidth = ImGui::CalcTextSize(label).x;
		ImDrawList& drawList = *ImGui::GetWindowDrawList();
		const ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
		DrawActionIcon(drawList, action, ImVec2{centerX, min.y + iconTop}, color);
		drawList.AddText(ImVec2{centerX - labelWidth * 0.5f, min.y + labelTop}, color, label);
		ImGui::PopID();
		return clicked;
	}

	void
	LauncherLayout::DrawActionIcon(ImDrawList& drawList, ProjectAction action, const ImVec2& center, ImU32 color) const
	{
		if (action == ProjectAction::New)
		{
			constexpr float radius = 9.0f;
			drawList
			    .AddLine(ImVec2{center.x - radius, center.y}, ImVec2{center.x + radius, center.y}, color, g_IconStroke);
			drawList
			    .AddLine(ImVec2{center.x, center.y - radius}, ImVec2{center.x, center.y + radius}, color, g_IconStroke);
			return;
		}
		DrawFolderIcon(drawList, center, color, 1.0f);
	}

	void LauncherLayout::DrawFolderIcon(ImDrawList& drawList, const ImVec2& center, ImU32 color, float scale) const
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

}
