#include "rdpch.h"

#include "Project.h"

namespace Rod {

	Project::Project(const std::filesystem::path& manifestPath, ProjectProperties properties)
	    : m_ManifestPath(manifestPath), m_Properties(std::move(properties))
	{}

	std::filesystem::path Project::GetAssetsPath() const
	{
		if (m_Properties.AssetsPath.empty())
			return {};
		return (GetDirectory() / m_Properties.AssetsPath).lexically_normal();
	}

	std::filesystem::path Project::GetStartupScenePath() const
	{
		if (m_Properties.StartupScenePath.empty())
			return {};
		return (GetDirectory() / m_Properties.StartupScenePath).lexically_normal();
	}

	std::filesystem::path Project::GetScriptProjectPath() const
	{
		if (m_Properties.ScriptsProjectPath.empty())
			return {};
		return (GetDirectory() / m_Properties.ScriptsProjectPath).lexically_normal();
	}

}
