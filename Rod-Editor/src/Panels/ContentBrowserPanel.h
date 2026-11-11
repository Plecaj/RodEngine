#pragma once

#include "Rod/Renderer/Texture.h"

#include <filesystem>

struct ImVec2;

namespace Rod {

	class ContentBrowserPanel
	{
	public:
		explicit ContentBrowserPanel(const std::filesystem::path& assetsPath);

		void OnImGuiRender();

	private:
		static bool DrawBackButton(const char* id, const ImVec2& size);
		void DrawNavigationBar();
		void SetupGridLayout(float thumbnailSize, float padding);
		void DrawDirectoryContents(float thumbnailSize);
		void DrawIconButton(const std::string& id, const Texture2D& icon, float size);
		void HandleDragDrop(
		    const std::filesystem::path& path,
		    const std::filesystem::path& relativePath,
		    const std::string& filename);
		void BeginAssetDragDrop(
		    const char* payloadType,
		    const std::filesystem::path& relativePath,
		    const std::string& filename);

	private:
		std::filesystem::path m_AssetsPath;
		std::filesystem::path m_CurrentDirectory;

		Scope<Texture2D> m_DirectoryIcon, m_FileIcon;
	};

}
