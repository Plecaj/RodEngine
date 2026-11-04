#include "rdpch.h"

#include "Rod/Utils/Process.h"

// NOLINTBEGIN(misc-include-cleaner)
#include <Windows.h>

namespace Rod {

	struct HandleDeleter
	{
		void operator()(void* handle) const { CloseHandle(handle); }
	};

	using Handle = std::unique_ptr<void, HandleDeleter>;

	static std::wstring QuoteArgument(const std::string& argument)
	{
		constexpr size_t quoteEscapeMultiplier = 2;
		int size = MultiByteToWideChar(
		    CP_UTF8,
		    MB_ERR_INVALID_CHARS,
		    argument.data(),
		    static_cast<int>(argument.size()),
		    nullptr,
		    0);
		std::wstring value(size, L'\0');
		MultiByteToWideChar(
		    CP_UTF8,
		    MB_ERR_INVALID_CHARS,
		    argument.data(),
		    static_cast<int>(argument.size()),
		    value.data(),
		    size);

		std::wstring quoted = L"\"";
		size_t backslashes = 0;
		for (wchar_t character : value)
		{
			if (character == L'\\')
			{
				++backslashes;
				continue;
			}
			quoted.append(character == L'"' ? backslashes * quoteEscapeMultiplier + 1 : backslashes, L'\\');
			quoted += character;
			backslashes = 0;
		}
		quoted.append(backslashes * quoteEscapeMultiplier, L'\\');
		return quoted + L'"';
	}

	static std::string ReadPipe(HANDLE readPipe)
	{
		constexpr size_t outputBufferSize = 4096;
		char buffer[outputBufferSize];
		DWORD count;
		std::string output;
		while (ReadFile(readPipe, buffer, sizeof(buffer), &count, nullptr) && count > 0)
			output.append(buffer, count);
		return output;
	}

	ProcessResult Process::Run(const std::vector<std::string>& arguments, bool captureErrors)
	{
		if (arguments.empty())
			return {-1, "No executable specified."};

		std::wstring command;
		for (const auto& argument : arguments)
		{
			if (!command.empty())
				command += L' ';
			command += QuoteArgument(argument);
		}

		SECURITY_ATTRIBUTES security = {sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
		HANDLE readPipe, writePipe;
		if (!CreatePipe(&readPipe, &writePipe, &security, 0))
			return {-1, "Failed to create process output pipe."};
		Handle reader(readPipe), writer(writePipe);
		if (!SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0))
			return {-1, "Failed to configure process output pipe."};

		STARTUPINFOW startup = {};
		startup.cb = sizeof(startup);
		startup.dwFlags = STARTF_USESTDHANDLES;
		startup.hStdOutput = writePipe;
		startup.hStdError = captureErrors ? writePipe : GetStdHandle(STD_ERROR_HANDLE);
		startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		PROCESS_INFORMATION information = {};
		if (CreateProcessW(
		        nullptr,
		        command.data(),
		        nullptr,
		        nullptr,
		        TRUE,
		        CREATE_NO_WINDOW,
		        nullptr,
		        nullptr,
		        &startup,
		        &information))
			return {-1, "Failed to start process: " + std::to_string(GetLastError())};

		Handle process(information.hProcess), thread(information.hThread);
		writer.reset();
		ProcessResult result;
		result.Output = ReadPipe(readPipe);

		if (WaitForSingleObject(process.get(), INFINITE) == WAIT_OBJECT_0)
		{
			DWORD exitCode;
			if (GetExitCodeProcess(process.get(), &exitCode))
				result.ExitCode = static_cast<int>(exitCode);
		}
		return result;
	}

}
// NOLINTEND(misc-include-cleaner)
