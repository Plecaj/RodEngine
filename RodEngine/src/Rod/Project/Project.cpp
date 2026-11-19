#include "rdpch.h"

#include "Project.h"

namespace Rod {

	Project::Project(std::filesystem::path manifestPath, ProjectProperties properties) noexcept
	    : m_ManifestPath(std::move(manifestPath)), m_Properties(std::move(properties))
	{}

	auto Project::GetAssetsPath() const -> std::filesystem::path
	{
		if (m_Properties.AssetsPath.empty())
			return {};
		return (GetDirectory() / m_Properties.AssetsPath).lexically_normal();
	}

	auto Project::GetStartupScenePath() const -> std::filesystem::path
	{
		if (m_Properties.StartupScenePath.empty())
			return {};
		return (GetDirectory() / m_Properties.StartupScenePath).lexically_normal();
	}

	auto Project::GetScriptProjectPath() const -> std::filesystem::path
	{
		if (m_Properties.ScriptsProjectPath.empty())
			return {};
		return (GetDirectory() / m_Properties.ScriptsProjectPath).lexically_normal();
	}

	auto Project::GetShadersPath() const -> std::filesystem::path
	{
		if (m_Properties.ShadersPath.empty())
			return {};
		return (GetDirectory() / m_Properties.ShadersPath).lexically_normal();
	}

}
