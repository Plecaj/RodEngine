#pragma once

#include "Rod/Core/Log.h"
#include "Rod/Project/Project.h"

#include <cstdio>
#include <exception>
#include <optional>
#include <print>
#include <utility>

namespace Rod {

	[[nodiscard]] auto ReadProject(int argc, char** argv) -> std::optional<Project>;

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
			try
			{
				std::print(stderr, "RodEngine failed: {}\n", error.what());
			}
			catch (...)
			{
				std::fputs("RodEngine failed.\n", stderr);
			}
		}
		catch (...)
		{
			std::fputs("RodEngine failed: unknown exception\n", stderr);
		}
		return 1;
	}

}
