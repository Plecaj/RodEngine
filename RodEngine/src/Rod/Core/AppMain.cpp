#include "rdpch.h"

#include "AppMain.h"

#include "Rod/Project/ProjectSerializer.h"

#include <span>
#include <stdexcept>

namespace Rod {

	auto ReadProject(int argc, char** argv) -> std::optional<Project>
	{
		if (argc <= 1)
			return std::nullopt;
		if (argc != 2)
			throw std::invalid_argument("Expected a single .rodproj manifest argument");

		const std::span arguments(argv, static_cast<size_t>(argc));
		Project project{std::filesystem::absolute(arguments.back()).lexically_normal(), {}};
		ProjectSerializer::Deserialize(project);
		return project;
	}

}
