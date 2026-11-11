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
		[[nodiscard]] std::filesystem::path GetAssetsPath() const;
		[[nodiscard]] std::filesystem::path GetStartupScenePath() const;
		[[nodiscard]] std::filesystem::path GetScriptProjectPath() const;

		void SetProperties(ProjectProperties properties) { m_Properties = std::move(properties); };

		static constexpr int GetRequiredMembersCount() { return MEMBERS_COUNT; }

	private:
		std::filesystem::path m_ManifestPath;
		ProjectProperties m_Properties;

		static constexpr int MEMBERS_COUNT = ProjectProperties::PROPERTIES_COUNT + 1;
	};

}
