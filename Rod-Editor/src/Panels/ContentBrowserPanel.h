#pragma once

#include <filesystem>
#include "Rod/Renderer/Texture.h"

namespace Rod{

	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();
	private:
		void DrawNavigationBar();
		void SetupGridLayout(float thumbnailSize, float padding);
		void DrawDirectoryContents(float thumbnailSize);
		void DrawIconButton(const std::string& id, Ref<Texture2D> icon, float size);
		void HandleDragDrop(const std::filesystem::path& path, const std::filesystem::path& relativePath, const std::string& filename);

	private:
		std::filesystem::path m_CurrentDirectory;

		Ref<Texture2D> m_DirectoryIcon, m_FileIcon;
	};

}