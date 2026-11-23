#include "rdpch.h"

#include "AppMain.h"

#include "Rod/Project/ProjectSerializer.h"

#include <stdexcept>

namespace Rod {

	auto ReadProject(int argc, char* argv[]) -> std::optional<Project>
	{
		if (argc <= 1)
			return std::nullopt;
		if (argc != 2)
			throw std::invalid_argument("Expected a single .rodproj manifest argument");

		Project project{std::filesystem::absolute(argv[1]).lexically_normal(), {}};
		ProjectSerializer::Deserialize(project);
		return project;
	}

}
