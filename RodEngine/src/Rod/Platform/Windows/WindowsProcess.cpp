#include "rdpch.h"

#include "Rod/Utils/Process.h"

// NOLINTBEGIN(misc-include-cleaner)
#include <Windows.h>

#include <array>

namespace Rod {

	namespace {
		struct HandleDeleter
		{
			void operator()(void* handle) const { CloseHandle(handle); }
		};

		using Handle = std::unique_ptr<void, HandleDeleter>;

		auto QuoteArgument(const std::string& argument) -> std::wstring
		{
			constexpr size_t quoteEscapeMultiplier = 2;
			const int size = MultiByteToWideChar(
			    CP_UTF8,
			    MB_ERR_INVALID_CHARS,
			    argument.data(),
			    static_cast<int>(argument.size()),
			    nullptr,
			    0);
			std::wstring value(static_cast<std::wstring::size_type>(size), L'\0');
			MultiByteToWideChar(
			    CP_UTF8,
			    MB_ERR_INVALID_CHARS,
			    argument.data(),
			    static_cast<int>(argument.size()),
			    value.data(),
			    size);

			std::wstring quoted = L"\"";
			size_t backslashes = 0;
			for (const wchar_t character : value)
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

		auto BuildCommand(const std::vector<std::string>& arguments) -> std::wstring
		{
			std::wstring command;
			for (const auto& argument : arguments)
			{
				if (!command.empty())
					command += L' ';
				command += QuoteArgument(argument);
			}
			return command;
		}

		auto ReadPipe(HANDLE readPipe) -> std::string
		{
			constexpr size_t outputBufferSize = 4096;
			std::array<char, outputBufferSize> buffer{};
			DWORD count = 0;
			std::string output;
			while (ReadFile(readPipe, buffer.data(), static_cast<DWORD>(buffer.size()), &count, nullptr) && count > 0)
				output.append(buffer.data(), count);
			return output;
		}
	}

	auto Process::Start(const std::vector<std::string>& arguments) -> ProcessResult
	{
		if (arguments.empty())
			return {.ExitCode = -1, .Output = "No executable specified."};

		auto command = BuildCommand(arguments);
		STARTUPINFOW startup = {};
		startup.cb = sizeof(startup);
		PROCESS_INFORMATION information = {};
		if (!CreateProcessW(
		        nullptr,
		        command.data(),
		        nullptr,
		        nullptr,
		        FALSE,
		        CREATE_NO_WINDOW,
		        nullptr,
		        nullptr,
		        &startup,
		        &information))
			return {.ExitCode = -1, .Output = "Failed to start process: " + std::to_string(GetLastError())};
		const Handle process(information.hProcess), thread(information.hThread);
		return {.ExitCode = 0, .Output = {}};
	}

	auto Process::Run(const std::vector<std::string>& arguments, bool captureErrors) -> ProcessResult
	{
		if (arguments.empty())
			return {.ExitCode = -1, .Output = "No executable specified."};

		auto command = BuildCommand(arguments);

		SECURITY_ATTRIBUTES security{
		    .nLength = sizeof(SECURITY_ATTRIBUTES),
		    .lpSecurityDescriptor = nullptr,
		    .bInheritHandle = TRUE};
		HANDLE readPipe = nullptr, writePipe = nullptr;
		if (!CreatePipe(&readPipe, &writePipe, &security, 0))
			return {.ExitCode = -1, .Output = "Failed to create process output pipe."};
		const Handle reader(readPipe);
		Handle writer(writePipe);
		if (!SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0))
			return {.ExitCode = -1, .Output = "Failed to configure process output pipe."};

		STARTUPINFOW startup = {};
		startup.cb = sizeof(startup);
		startup.dwFlags = STARTF_USESTDHANDLES;
		startup.hStdOutput = writePipe;
		startup.hStdError = captureErrors ? writePipe : GetStdHandle(STD_ERROR_HANDLE);
		startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		PROCESS_INFORMATION information = {};
		if (!CreateProcessW(
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
			return {.ExitCode = -1, .Output = "Failed to start process: " + std::to_string(GetLastError())};

		const Handle process(information.hProcess), thread(information.hThread);
		writer.reset();
		ProcessResult result;
		result.Output = ReadPipe(readPipe);

		if (WaitForSingleObject(process.get(), INFINITE) == WAIT_OBJECT_0)
		{
			DWORD exitCode = 0;
			if (GetExitCodeProcess(process.get(), &exitCode))
				result.ExitCode = static_cast<int>(exitCode);
		}
		return result;
	}

}
// NOLINTEND(misc-include-cleaner)
