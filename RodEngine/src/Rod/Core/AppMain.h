#pragma once

#include "Rod/Core/Log.h"
#include "Rod/Project/Project.h"

#include <cstdio>
#include <exception>
#include <optional>
#include <utility>

namespace Rod {

	[[nodiscard]] auto ReadProject(int argc, char* argv[]) -> std::optional<Project>;

	template <typename Run>
	auto RunMain(Run&& run) -> int
	{
		try
		{
			Log::Init();
			std::forward<Run>(run)();
			return 0;
		}
		catch (const std::exception& error)
		{
			std::fprintf(stderr, "RodEngine failed: %s\n", error.what());
		}
		catch (...)
		{
			std::fputs("RodEngine failed: unknown exception\n", stderr);
		}
		return 1;
	}

}
