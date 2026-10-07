#include "rdpch.h"
#include "Rod/Utils/Process.h"

#include <Windows.h>

namespace Rod {

	struct HandleDeleter
	{
		void operator()(void* handle) const { CloseHandle(handle); }
	};

	using Handle = std::unique_ptr<void, HandleDeleter>;

	static std::wstring QuoteArgument(const std::string& argument)
	{
		int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, argument.data(), (int)argument.size(), nullptr, 0);
		std::wstring value(size, L'\0');
		MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, argument.data(), (int)argument.size(), value.data(), size);

		std::wstring quoted = L"\"";
		size_t backslashes = 0;
		for (wchar_t character : value)
		{
			if (character == L'\\')
			{
				++backslashes;
				continue;
			}
			quoted.append(character == L'"' ? backslashes * 2 + 1 : backslashes, L'\\');
			quoted += character;
			backslashes = 0;
		}
		quoted.append(backslashes * 2, L'\\');
		return quoted + L'"';
	}

	ProcessResult Process::Run(const std::vector<std::string>& arguments, bool captureErrors)
	{
		if (arguments.empty())
			return { -1, "No executable specified." };

		std::wstring command;
		for (const auto& argument : arguments)
		{
			if (!command.empty())
				command += L' ';
			command += QuoteArgument(argument);
		}

		SECURITY_ATTRIBUTES security = { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
		HANDLE readPipe, writePipe;
		if (!CreatePipe(&readPipe, &writePipe, &security, 0))
			return { -1, "Failed to create process output pipe." };
		Handle reader(readPipe), writer(writePipe);
		if (!SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0))
			return { -1, "Failed to configure process output pipe." };

		STARTUPINFOW startup = {};
		startup.cb = sizeof(startup);
		startup.dwFlags = STARTF_USESTDHANDLES;
		startup.hStdOutput = writePipe;
		startup.hStdError = captureErrors ? writePipe : GetStdHandle(STD_ERROR_HANDLE);
		startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		PROCESS_INFORMATION information = {};
		if (!CreateProcessW(nullptr, command.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &startup, &information))
			return { -1, "Failed to start process: " + std::to_string(GetLastError()) };

		Handle process(information.hProcess), thread(information.hThread);
		writer.reset();
		ProcessResult result;
		char buffer[4096];
		DWORD count;
		while (ReadFile(readPipe, buffer, sizeof(buffer), &count, nullptr) && count > 0)
			result.Output.append(buffer, count);

		if (WaitForSingleObject(process.get(), INFINITE) == WAIT_OBJECT_0)
		{
			DWORD exitCode;
			if (GetExitCodeProcess(process.get(), &exitCode))
				result.ExitCode = (int)exitCode;
		}
		return result;
	}

}
