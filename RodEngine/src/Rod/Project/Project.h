#pragma once

#include "ProjectProperties.h"

#include <filesystem>
#include <utility>

namespace Rod {

	class Project
	{
	public:
		Project(std::filesystem::path manifestPath, ProjectProperties properties) noexcept;

		[[nodiscard]] auto GetProperties() const noexcept -> const ProjectProperties& { return m_Properties; }
		[[nodiscard]] auto GetManifestPath() const noexcept -> const std::filesystem::path& { return m_ManifestPath; }
		[[nodiscard]] auto GetDirectory() const -> std::filesystem::path { return m_ManifestPath.parent_path(); }
		[[nodiscard]] auto GetAssetsPath() const -> std::filesystem::path;
		[[nodiscard]] auto GetStartupScenePath() const -> std::filesystem::path;
		[[nodiscard]] auto GetScriptProjectPath() const -> std::filesystem::path;
		[[nodiscard]] auto GetShadersPath() const -> std::filesystem::path;

		void SetProperties(ProjectProperties properties) noexcept { m_Properties = std::move(properties); };

	private:
		std::filesystem::path m_ManifestPath;
		ProjectProperties m_Properties;
	};

}
