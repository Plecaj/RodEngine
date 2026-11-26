#include "rdpch.h"

#include "LauncherApp.h"

#include "Rod/Core/DesktopHost.h"
#include "Rod/Project/Project.h"
#include "Rod/Project/ProjectSerializer.h"
#include "Rod/Utils/Process.h"

#include <exception>
#include <stdexcept>

namespace Rod {

	LauncherApp::LauncherApp(DesktopHost& host, const RenderDevice& device)
	    : m_Host(host), m_FileDialogs(host.GetWindow()), m_Layout(m_FileDialogs, device)
	{
		try
		{
			m_RecentProjectsPath = Platform::GetExecutablePath().parent_path() / "RecentProjects.yaml";
			m_RecentProjects.Load(m_RecentProjectsPath);
		}
		catch (const std::exception& exception)
		{
			Log::Core().warn("Could not load recent projects: {}", exception.what());
		}
	}

	void LauncherApp::Draw()
	{
		m_Layout.OnImGuiRender(
		    m_RecentProjects,
		    {.OpenProject = [this](const std::filesystem::path& path) { return OpenProject(path); },
			 .LaunchEditor = [this](const Project& project) { return LaunchEditor(project); }});
	}

	auto LauncherApp::OpenProject(std::filesystem::path manifestPath) -> std::optional<std::string>
	{
		try
		{
			if (manifestPath.empty())
				manifestPath = m_FileDialogs.OpenFile("Rod Project (*.rodproj)\0*.rodproj\0");
			if (manifestPath.empty())
				return std::nullopt;

			Project project{std::filesystem::absolute(manifestPath).lexically_normal(), {}};
			ProjectSerializer::Deserialize(project);
			return LaunchEditor(project);
		}
		catch (const std::exception& exception)
		{
			return exception.what();
		}
	}

	auto LauncherApp::LaunchEditor(const Project& project) -> std::optional<std::string>
	{
		try
		{
			const auto executable = std::filesystem::current_path() / "Rod-Editor" ROD_EXECUTABLE_SUFFIX;
			const auto result = Process::Start({executable.string(), project.GetManifestPath().string()});
			if (result.ExitCode != 0)
				throw std::runtime_error("Could not launch editor: " + result.Output);

			try
			{
				m_RecentProjects.Record(project);
				m_RecentProjects.Save(m_RecentProjectsPath);
			}
			catch (const std::exception& exception)
			{
				Log::Core().warn("Could not save recent projects: {}", exception.what());
			}
			m_Host.RequestClose();
			return std::nullopt;
		}
		catch (const std::exception& exception)
		{
			return exception.what();
		}
	}

}
