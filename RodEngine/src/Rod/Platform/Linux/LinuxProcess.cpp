#include "rdpch.h"

#include "Rod/Utils/Process.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h> // IWYU pragma: keep
#include <future>
#include <spawn.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

namespace Rod {

	static std::vector<char*> BuildArguments(const std::vector<std::string>& arguments)
	{
		std::vector<char*> argv;
		argv.reserve(arguments.size() + 1);
		for (const auto& argument : arguments)
		{
			// POSIX borrows these strings without modifying them, despite the mutable argv type.
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
			argv.push_back(const_cast<char*>(argument.c_str()));
		}
		argv.push_back(nullptr);
		return argv;
	}

	ProcessResult Process::Start(const std::vector<std::string>& arguments)
	{
		if (arguments.empty())
			return {-1, "No executable specified."};

		auto argv = BuildArguments(arguments);
		std::promise<pid_t> child;
		std::thread([process = child.get_future()]() mutable {
			const auto pid = process.get();
			if (pid > 0)
			{
				while (waitpid(pid, nullptr, 0) == -1 && errno == EINTR)
				{}
			}
		}).detach();
		pid_t process = 0;
		const int error = posix_spawnp(&process, argv[0], nullptr, nullptr, argv.data(), environ);
		child.set_value(error == 0 ? process : 0);
		if (error != 0)
			return {-1, std::strerror(error)};
		return {0, {}};
	}

	ProcessResult Process::Run(const std::vector<std::string>& arguments, bool captureErrors)
	{
		if (arguments.empty())
			return {-1, "No executable specified."};

		auto argv = BuildArguments(arguments);

		constexpr size_t pipeEndCount = 2;
		std::array<int, pipeEndCount> output;
		if (pipe2(output.data(), O_CLOEXEC) != 0)
			return {-1, std::strerror(errno)};

		posix_spawn_file_actions_t actions;
		int error = posix_spawn_file_actions_init(&actions);
		if (error != 0)
		{
			close(output[0]);
			close(output[1]);
			return {-1, std::strerror(error)};
		}

		error = posix_spawn_file_actions_adddup2(&actions, output[1], STDOUT_FILENO);
		if (error == 0)
			error = captureErrors ? posix_spawn_file_actions_adddup2(&actions, output[1], STDERR_FILENO)
			                      : posix_spawn_file_actions_addopen(&actions, STDERR_FILENO, "/dev/null", O_WRONLY, 0);

		pid_t process = 0;
		if (error == 0)
			error = posix_spawnp(&process, argv[0], &actions, nullptr, argv.data(), environ);
		posix_spawn_file_actions_destroy(&actions);
		close(output[1]);

		ProcessResult result;
		if (error != 0)
		{
			close(output[0]);
			result.Output = std::strerror(error);
			return result;
		}

		constexpr size_t outputBufferSize = 4096;
		std::array<char, outputBufferSize> buffer;
		ssize_t count{};
		while ((count = read(output[0], buffer.data(), buffer.size())) != 0)
		{
			if (count > 0)
				result.Output.append(buffer.data(), static_cast<size_t>(count));
			else if (errno != EINTR)
				break;
		}
		close(output[0]);

		int status = 0;
		pid_t waited{};
		do
		{
			waited = waitpid(process, &status, 0);
		} while (waited == -1 && errno == EINTR);

		if (waited != -1 && WIFEXITED(status))
			result.ExitCode = WEXITSTATUS(status);
		return result;
	}

}
