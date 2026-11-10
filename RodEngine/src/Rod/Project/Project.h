#pragma once

#include "ProjectProperties.h"

#include <filesystem>
#include <utility>

namespace Rod {

	class Project
	{
	public:
		Project(const std::filesystem::path& manifestPath, ProjectProperties properties);

		[[nodiscard]] const ProjectProperties& GetProperties() const { return m_Properties; }
		[[nodiscard]] const std::filesystem::path& GetManifestPath() const { return m_ManifestPath; }
		[[nodiscard]] std::filesystem::path GetDirectory() const { return m_ManifestPath.parent_path(); }
		[[nodiscard]] std::filesystem::path GetAssetsPath() const { return m_Properties.AssetsPath; };
		[[nodiscard]] std::filesystem::path GetStartupScenePath() const { return m_Properties.StartupScenePath; };
		[[nodiscard]] std::filesystem::path GetScriptProjectPath() const { return m_Properties.ScriptsProjectPath; };

		void SetProperties(ProjectProperties properties) { m_Properties = std::move(properties); };

	private:
		std::filesystem::path m_ManifestPath;
		ProjectProperties m_Properties;
	};

}
