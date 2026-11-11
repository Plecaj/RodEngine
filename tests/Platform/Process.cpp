#include "rdpch.h"

#include "Rod/Utils/Process.h"

#include "Rod/Core/UUID.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <fstream>
#include <iterator>
#include <system_error>
#include <thread>

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

TEST_CASE("Windows process starter reports launch failures", "[process]")
{
	const auto result = Rod::Process::Start({"RodEngine-missing-test-executable.exe"});
	REQUIRE(result.ExitCode == -1);
	REQUIRE(result.Output.starts_with("Failed to start process: "));
	REQUIRE(Rod::Process::Start({}).ExitCode == -1);
}

TEST_CASE("Windows process starter returns before exit and preserves project arguments", "[process]")
{
	struct TemporaryDirectory
	{
		TemporaryDirectory()
		{
			do
			{
				Path = std::filesystem::temp_directory_path() /
				       ("rod-process-" + std::to_string(static_cast<uint64_t>(Rod::UUID{})));
			} while (!std::filesystem::create_directory(Path));
		}
		~TemporaryDirectory()
		{
			std::error_code error;
			std::filesystem::remove_all(Path, error);
		}
		std::filesystem::path Path;
	} directory;
	const auto output = directory.Path / "arguments.bin";
	const auto release = directory.Path / "release";
	const std::vector<std::string> projectArguments{
	    R"(C:\Projects\My Game\project.rdproj)",
	    R"(Game "one")",
	    R"(Assets with spaces\)",
	    "",
	    R"(Scripts\Game.csproj)"};
	std::vector<std::string> command{ROD_PROCESS_TEST_HELPER_PATH, output.string(), release.string()};
	command.insert(command.end(), projectArguments.begin(), projectArguments.end());
	const auto result = Rod::Process::Start(command);
	REQUIRE(result.ExitCode == 0);
	REQUIRE(result.Output.empty());

	// The child cannot finish until Start has returned and this file is created.
	std::ofstream(release).close();
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
	while (!std::filesystem::exists(output) && std::chrono::steady_clock::now() < deadline)
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	REQUIRE(std::filesystem::exists(output));
	std::ifstream file(output, std::ios::binary);
	const std::string actual{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
	std::string expected;
	for (const auto& argument : projectArguments)
	{
		expected += argument;
		expected += '\0';
	}
	REQUIRE(actual == expected);
}
