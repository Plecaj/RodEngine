#pragma once

#include "Rod/Renderer/Texture.h"

#include <filesystem>

namespace Rod {

	class ContentBrowserPanel
	{
	public:
		explicit ContentBrowserPanel(const std::filesystem::path& assetsPath);

		void OnImGuiRender();

	private:
		void DrawNavigationBar();
		void DrawDirectoryContents(float thumbnailSize);

	private:
		std::filesystem::path m_AssetsPath;
		std::filesystem::path m_CurrentDirectory;

		Scope<Texture2D> m_DirectoryIcon, m_FileIcon;
	};

}
