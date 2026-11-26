#pragma once

#include "LauncherLayout.h"
#include "RecentProjects.h"
#include "Rod/Utils/PlatformUtils.h"

#include <filesystem>
#include <optional>
#include <string>

namespace Rod {

	class DesktopHost;
	class Project;
	class RenderDevice;

	class LauncherApp
	{
	public:
		LauncherApp(DesktopHost& host, const RenderDevice& device);
		LauncherApp(const LauncherApp&) = delete;
		auto operator=(const LauncherApp&) -> LauncherApp& = delete;
		LauncherApp(LauncherApp&&) = delete;
		auto operator=(LauncherApp&&) -> LauncherApp& = delete;
		~LauncherApp() = default;

		void Draw();

	private:
		[[nodiscard]] auto OpenProject(std::filesystem::path manifestPath) -> std::optional<std::string>;
		[[nodiscard]] auto LaunchEditor(const Project& project) -> std::optional<std::string>;

		DesktopHost& m_Host;
		FileDialogs m_FileDialogs;
		RecentProjects m_RecentProjects;
		std::filesystem::path m_RecentProjectsPath;
		LauncherLayout m_Layout;
	};

}
