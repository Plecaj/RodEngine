#include "rdpch.h"

#include "Project.h"

namespace Rod {

	Project::Project(const std::filesystem::path& manifestPath, ProjectProperties properties)
	    : m_ManifestPath(manifestPath), m_Properties(std::move(properties))
	{}

}
