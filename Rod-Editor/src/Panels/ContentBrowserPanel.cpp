#include "rdpch.h"
#include "ContentBrowserPanel.h"

#include <imgui.h>

namespace Rod {

    extern const std::filesystem::path g_AssetsPath = "assets";

    static bool DrawBackButton(const char* id, const ImVec2& size)
    {
        ImGui::InvisibleButton(id, size);

        bool hovered = ImGui::IsItemHovered();
        bool active = ImGui::IsItemActive();

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 min = ImGui::GetItemRectMin();
        ImVec2 max = ImGui::GetItemRectMax();
        ImU32 background = active ? IM_COL32(31, 35, 42, 255) : hovered ? IM_COL32(42, 48, 57, 255) : IM_COL32(25, 29, 35, 255);
        ImU32 arrow = IM_COL32(224, 229, 236, 255);

        drawList->AddRectFilled(min, max, background, 4.0f);

        ImVec2 center = ImVec2((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f);
        float width = 13.0f;
        float height = 9.0f;
        drawList->AddLine(ImVec2(center.x - width * 0.5f, center.y), ImVec2(center.x + width * 0.5f, center.y), arrow, 1.8f);
        drawList->AddLine(ImVec2(center.x - width * 0.5f, center.y), ImVec2(center.x - width * 0.5f + height * 0.65f, center.y - height * 0.5f), arrow, 1.8f);
        drawList->AddLine(ImVec2(center.x - width * 0.5f, center.y), ImVec2(center.x - width * 0.5f + height * 0.65f, center.y + height * 0.5f), arrow, 1.8f);

        return ImGui::IsItemClicked(ImGuiMouseButton_Left);
    }

    ContentBrowserPanel::ContentBrowserPanel()
        : m_CurrentDirectory(g_AssetsPath)
    {
        m_DirectoryIcon = Texture2D::Create("assets/textures/DirectoryIcon.png");
        m_FileIcon = Texture2D::Create("assets/textures/FileIcon.png");
    }

    void ContentBrowserPanel::OnImGuiRender()
    {
        ImGui::Begin("Content Browser");

        DrawNavigationBar();

        float thumbnailSize = 72.0f;
        float padding = 20.0f;
        SetupGridLayout(thumbnailSize, padding);

        DrawDirectoryContents(thumbnailSize);

        ImGui::Columns(1);
        ImGui::End();
    }

    void ContentBrowserPanel::DrawNavigationBar()
    {
        if (m_CurrentDirectory != g_AssetsPath)
        {
            if (DrawBackButton("##ContentBrowserBack", ImVec2(34.0f, 28.0f)))
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            ImGui::SameLine(0.0f, 10.0f);
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);
        ImGui::TextDisabled("%s", m_CurrentDirectory.string().c_str());
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 4.0f));
    }

    void ContentBrowserPanel::SetupGridLayout(float thumbnailSize, float padding)
    {
        float cellSize = thumbnailSize + padding;

        ImVec2 panelSize = ImGui::GetContentRegionAvail();
        int columnCount = (int)(panelSize.x / cellSize);
        if (columnCount < 1) columnCount = 1;

        ImGui::Columns(columnCount, 0, false);
    }

    void ContentBrowserPanel::DrawDirectoryContents(float thumbnailSize)
    {
        for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
        {
            const auto& path = directoryEntry.path();
            auto relativePath = std::filesystem::relative(path, g_AssetsPath);
            std::string filenameString = path.filename().string();

            std::string uniqueID = relativePath.string();
            ImGui::PushID(uniqueID.c_str());

            Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;

            DrawIconButton(uniqueID, icon, thumbnailSize);
            HandleDragDrop(path, relativePath, filenameString);

            if (directoryEntry.is_directory() && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                m_CurrentDirectory /= path.filename();
            }

            ImGui::TextWrapped("%s", filenameString.c_str());
            ImGui::NextColumn();

            ImGui::PopID();
        }
    }

    void ContentBrowserPanel::DrawIconButton(const std::string& id, Ref<Texture2D> icon, float size)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.11f, 0.13f, 0.16f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.21f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.14f, 0.16f, 0.20f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 8.0f));
        ImGui::ImageButton(
            id.c_str(),
            (ImTextureID)(uint64_t)icon->GetRendererID(),
            { size, size },
            { 0.0f, 1.0f }, { 1.0f, 0.0f }
        );
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
    }

    void ContentBrowserPanel::HandleDragDrop(const std::filesystem::path& path, const std::filesystem::path& relativePath, const std::string& filename)
    {
        if (path.extension() == ".rod")
        {
            if (ImGui::BeginDragDropSource())
            {
                std::string itemPath = relativePath.string();
                ImGui::SetDragDropPayload("CONTENT_BROWSER_SCENE_ITEM", itemPath.c_str(), itemPath.size() + 1);
                ImGui::Text("%s", filename.c_str());
                ImGui::EndDragDropSource();
            }
        }
        else if (path.extension() == ".png")
        {
            if (ImGui::BeginDragDropSource())
            {
                std::string itemPath = relativePath.string();
                ImGui::SetDragDropPayload("CONTENT_BROWSER_TEXTURE_ITEM", itemPath.c_str(), itemPath.size() + 1);
                ImGui::Text("%s", filename.c_str());
                ImGui::EndDragDropSource();
            }
        }
        else if (path.extension() == ".glb")
        {
            if (ImGui::BeginDragDropSource())
            {
                std::string itemPath = relativePath.string();
                ImGui::SetDragDropPayload("CONTENT_BROWSER_MESH_ITEM", itemPath.c_str(), itemPath.size() + 1);
                ImGui::Text("%s", filename.c_str());
                ImGui::EndDragDropSource();
            }
        }
    }

}
