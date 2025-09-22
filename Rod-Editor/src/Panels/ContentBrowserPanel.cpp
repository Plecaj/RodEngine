#include "rdpch.h"
#include "ContentBrowserPanel.h"

#include <imgui.h>

namespace Rod {

    extern const std::filesystem::path g_AssetsPath = "assets";

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

        float thumbnailSize = 64.0f;
        float padding = 16.0f;
        SetupGridLayout(thumbnailSize, padding);

        DrawDirectoryContents(thumbnailSize);

        ImGui::Columns(1);
        ImGui::End();
    }

    void ContentBrowserPanel::DrawNavigationBar()
    {
        if (m_CurrentDirectory != g_AssetsPath)
        {
            if (ImGui::Button("<-"))
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
        }
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
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::ImageButton(
            id.c_str(),
            (ImTextureID)(uint64_t)icon->GetRendererID(),
            { size, size },
            { 0.0f, 1.0f }, { 1.0f, 0.0f }
        );
        ImGui::PopStyleColor();
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
    }

}
