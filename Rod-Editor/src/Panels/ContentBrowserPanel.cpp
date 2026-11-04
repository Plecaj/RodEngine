#include "rdpch.h"

#include "ContentBrowserPanel.h"

#include <imgui.h>

namespace Rod {

	extern const std::filesystem::path g_AssetsPath = "assets";

	bool ContentBrowserPanel::DrawBackButton(const char* id, const ImVec2& size)
	{
		constexpr float half = 0.5f;
		constexpr float cornerRadius = 4.0f;
		constexpr float arrowWingScale = 0.65f;
		constexpr float stroke = 1.8f;
		ImGui::InvisibleButton(id, size);

		const bool hovered = ImGui::IsItemHovered();
		const bool active = ImGui::IsItemActive();

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const ImVec2 min = ImGui::GetItemRectMin();
		const ImVec2 max = ImGui::GetItemRectMax();
		const ImU32 background = active    ? IM_COL32(31, 35, 42, 255)
		                         : hovered ? IM_COL32(42, 48, 57, 255)
		                                   : IM_COL32(25, 29, 35, 255);
		constexpr ImU32 arrow = IM_COL32(224, 229, 236, 255);

		drawList->AddRectFilled(min, max, background, cornerRadius);

		const ImVec2 center = ImVec2((min.x + max.x) * half, (min.y + max.y) * half);
		constexpr float width = 13.0f;
		constexpr float height = 9.0f;
		drawList->AddLine(
		    ImVec2(center.x - width * half, center.y),
		    ImVec2(center.x + width * half, center.y),
		    arrow,
		    stroke);
		drawList->AddLine(
		    ImVec2(center.x - width * half, center.y),
		    ImVec2(center.x - width * half + height * arrowWingScale, center.y - height * half),
		    arrow,
		    stroke);
		drawList->AddLine(
		    ImVec2(center.x - width * half, center.y),
		    ImVec2(center.x - width * half + height * arrowWingScale, center.y + height * half),
		    arrow,
		    stroke);

		return ImGui::IsItemClicked(ImGuiMouseButton_Left);
	}

	ContentBrowserPanel::ContentBrowserPanel() : m_CurrentDirectory(g_AssetsPath)
	{
		m_DirectoryIcon = Texture2D::Create("assets/textures/DirectoryIcon.png");
		m_FileIcon = Texture2D::Create("assets/textures/FileIcon.png");
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		DrawNavigationBar();

		constexpr float thumbnailSize = 72.0f;
		constexpr float padding = 20.0f;
		SetupGridLayout(thumbnailSize, padding);

		DrawDirectoryContents(thumbnailSize);

		ImGui::Columns(1);
		ImGui::End();
	}

	void ContentBrowserPanel::DrawNavigationBar()
	{
		constexpr ImVec2 backButtonSize{34.0f, 28.0f};
		constexpr float horizontalSpacing = 10.0f;
		constexpr float verticalSpacing = 4.0f;
		if (m_CurrentDirectory != g_AssetsPath)
		{
			if (DrawBackButton("##ContentBrowserBack", backButtonSize))
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			ImGui::SameLine(0.0f, horizontalSpacing);
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + verticalSpacing);
		ImGui::TextDisabled("%s", m_CurrentDirectory.string().c_str());
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Dummy(ImVec2(0.0f, verticalSpacing));
	}

	void ContentBrowserPanel::SetupGridLayout(float thumbnailSize, float padding)
	{
		const float cellSize = thumbnailSize + padding;

		const ImVec2 panelSize = ImGui::GetContentRegionAvail();
		const int columnCount = std::max(static_cast<int>(panelSize.x / cellSize), 1);

		ImGui::Columns(columnCount, nullptr, false);
	}

	void ContentBrowserPanel::DrawDirectoryContents(float thumbnailSize)
	{
		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, g_AssetsPath);
			const std::string filenameString = path.filename().string();

			const std::string uniqueID = relativePath.string();
			ImGui::PushID(uniqueID.c_str());

			const Texture2D& icon = directoryEntry.is_directory() ? *m_DirectoryIcon : *m_FileIcon;

			DrawIconButton(uniqueID, icon, thumbnailSize);
			HandleDragDrop(path, relativePath, filenameString);

			if (directoryEntry.is_directory() && ImGui::IsItemHovered() &&
			    ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				m_CurrentDirectory /= path.filename();
			}

			ImGui::TextWrapped("%s", filenameString.c_str());
			ImGui::NextColumn();

			ImGui::PopID();
		}
	}

	void ContentBrowserPanel::DrawIconButton(const std::string& id, const Texture2D& icon, float size)
	{
		constexpr auto buttonColors = std::to_array<std::pair<ImGuiCol, ImVec4>>(
		    {{ImGuiCol_Button, {0.11f, 0.13f, 0.16f, 1.0f}},
		     {ImGuiCol_ButtonHovered, {0.18f, 0.21f, 0.25f, 1.0f}},
		     {ImGuiCol_ButtonActive, {0.14f, 0.16f, 0.20f, 1.0f}}});
		constexpr ImVec2 framePadding{8.0f, 8.0f};
		for (const auto& [slot, color] : buttonColors)
			ImGui::PushStyleColor(slot, color);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, framePadding);
		ImGui::ImageButton(
		    id.c_str(),
		    static_cast<ImTextureID>(icon.GetRendererID()),
		    {size, size},
		    {0.0f, 1.0f},
		    {1.0f, 0.0f});
		ImGui::PopStyleVar();
		ImGui::PopStyleColor(static_cast<int>(buttonColors.size()));
	}

	void ContentBrowserPanel::HandleDragDrop(
	    const std::filesystem::path& path,
	    const std::filesystem::path& relativePath,
	    const std::string& filename)
	{
		if (path.extension() == ".rod")
			BeginAssetDragDrop("CONTENT_BROWSER_SCENE_ITEM", relativePath, filename);
		else if (path.extension() == ".png")
			BeginAssetDragDrop("CONTENT_BROWSER_TEXTURE_ITEM", relativePath, filename);
		else if (path.extension() == ".glb")
			BeginAssetDragDrop("CONTENT_BROWSER_MESH_ITEM", relativePath, filename);
	}

	void ContentBrowserPanel::BeginAssetDragDrop(
	    const char* payloadType,
	    const std::filesystem::path& relativePath,
	    const std::string& filename)
	{
		if (ImGui::BeginDragDropSource())
		{
			const std::string itemPath = relativePath.string();
			ImGui::SetDragDropPayload(payloadType, itemPath.c_str(), itemPath.size() + 1);
			ImGui::Text("%s", filename.c_str());
			ImGui::EndDragDropSource();
		}
	}

}
