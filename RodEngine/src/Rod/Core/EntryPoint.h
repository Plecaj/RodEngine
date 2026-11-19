#pragma once

#include "Rod/Project/Project.h"
#include "Rod/Project/ProjectSerializer.h"

#include <filesystem>
#include <optional>
#include <stdexcept>

namespace Utils {
	[[nodiscard]] inline auto GetProjectDataFromArguments(int argc, char* argv[]) -> std::optional<Rod::Project>
	{
		if (argc <= 1)
			return std::nullopt;
		constexpr int argumentCount = 2;
		if (argc != argumentCount)
			throw std::invalid_argument("Expected a single .rodproj manifest argument");

		Rod::Project project{std::filesystem::absolute(argv[1]).lexically_normal(), {}};
		Rod::ProjectSerializer::Deserialize(project);
		return project;
	}
}
