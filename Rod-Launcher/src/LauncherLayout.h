#pragma once

#include "ProjectSetupPanel.h"
#include "Rod/Renderer/Texture.h"

#include "imgui.h"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>

namespace Rod {

	class FileDialogs;
	class Project;
	class RecentProjects;
	class RenderDevice;
	struct RecentProject;

	class LauncherLayout
	{
	public:
		LauncherLayout(const FileDialogs& dialogs, const RenderDevice& device);

		struct Actions
		{
			std::function<std::optional<std::string>(const std::filesystem::path&)> OpenProject;
			std::function<std::optional<std::string>(const Project&)> LaunchEditor;
		};

		void OnImGuiRender(const RecentProjects& recentProjects, const Actions& actions);

	private:
		enum class ProjectAction : uint8_t
		{
			New,
			Open,
		};

		[[nodiscard]] auto DrawProjectActions() -> bool;
		[[nodiscard]] auto DrawRecentProjects(const RecentProjects& recentProjects) const
		    -> std::optional<std::filesystem::path>;
		[[nodiscard]] auto DrawRecentProject(const RecentProject& project) const -> bool;
		void ReportProjectError(std::optional<std::string> error);
		void DrawProjectResult() const;

		[[nodiscard]] auto DrawActionCard(const char* label, ProjectAction action) const -> bool;
		void DrawActionIcon(ImDrawList& drawList, ProjectAction action, const ImVec2& center, ImU32 color) const;
		void DrawFolderIcon(ImDrawList& drawList, const ImVec2& center, ImU32 color, float scale) const;

		ProjectSetupPanel m_ProjectSetup;
		Scope<Texture2D> m_FolderIcon;
		std::string m_Error;
	};

}
