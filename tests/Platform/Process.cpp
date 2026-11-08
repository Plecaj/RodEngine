#include "rdpch.h"

#include "Rod/Utils/Process.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Windows process runner reports child output and exit status", "[process]")
{
	const auto result = Rod::Process::Run({"where.exe", "cmd.exe"});
	REQUIRE(result.ExitCode == 0);
	REQUIRE(result.Output.find("cmd.exe") != std::string::npos);
}

TEST_CASE("Windows process runner reports launch failures", "[process]")
{
	const auto result = Rod::Process::Run({"RodEngine-missing-test-executable.exe"});
	REQUIRE(result.ExitCode == -1);
	REQUIRE(result.Output.starts_with("Failed to start process: "));
}
