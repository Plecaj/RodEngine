#include "rdpch.h"
#include "Rod/Utils/Process.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;

namespace Rod {

	ProcessResult Process::Run(const std::vector<std::string>& arguments, bool captureErrors)
	{
		if (arguments.empty())
			return { -1, "No executable specified." };

		std::vector<char*> argv;
		for (const auto& argument : arguments)
			argv.push_back(const_cast<char*>(argument.c_str()));
		argv.push_back(nullptr);

		int output[2];
		if (pipe2(output, O_CLOEXEC) != 0)
			return { -1, std::strerror(errno) };

		posix_spawn_file_actions_t actions;
		int error = posix_spawn_file_actions_init(&actions);
		if (error != 0)
		{
			close(output[0]);
			close(output[1]);
			return { -1, std::strerror(error) };
		}

		if ((error = posix_spawn_file_actions_adddup2(&actions, output[1], STDOUT_FILENO)) == 0)
			error = captureErrors
				? posix_spawn_file_actions_adddup2(&actions, output[1], STDERR_FILENO)
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

		char buffer[4096];
		ssize_t count;
		while ((count = read(output[0], buffer, sizeof(buffer))) != 0)
		{
			if (count > 0)
				result.Output.append(buffer, count);
			else if (errno != EINTR)
				break;
		}
		close(output[0]);

		int status = 0;
		pid_t waited;
		do
		{
			waited = waitpid(process, &status, 0);
		} while (waited == -1 && errno == EINTR);

		if (waited != -1 && WIFEXITED(status))
			result.ExitCode = WEXITSTATUS(status);
		return result;
	}

}
