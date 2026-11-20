#include "rdpch.h"

#include "Rod/Utils/PlatformUtils.h"
#include "Rod/Utils/Process.h"

#include <filesystem>

namespace Rod {

	auto Platform::GetExecutablePath() -> std::filesystem::path
	{
		return std::filesystem::read_symlink("/proc/self/exe");
	}

	static auto
	ShowFileDialog(const char* filter, bool save, const std::string& initialDirectory = {}, bool directory = false)
	    -> std::string
	{
		std::vector<std::string> arguments = {"zenity", "--file-selection"};
		if (directory)
			arguments.emplace_back("--directory");
		if (!initialDirectory.empty())
			arguments.push_back("--filename=" + (std::filesystem::path(initialDirectory) / "").string());
		if (save)
		{
			arguments.emplace_back("--save");
			arguments.emplace_back("--confirm-overwrite");
		}

		if (filter && *filter)
		{
			const std::string description = filter;
			const char* patterns = filter + description.size() + 1;
			if (*patterns)
			{
				std::string pattern = patterns;
				std::replace(pattern.begin(), pattern.end(), ';', ' ');
				arguments.push_back("--file-filter=" + description + " | " + pattern);
			}
		}

		auto result = Process::Run(arguments, false);
		if (result.ExitCode != 0)
		{
			if (result.ExitCode != 1)
				RD_CORE_ERROR("File dialog failed. Install zenity: {}", result.Output);
			return {};
		}

		if (!result.Output.empty() && result.Output.back() == '\n')
			result.Output.pop_back();
		return result.Output;
	}

	auto FileDialogs::OpenFile(const char* filter, const std::string& initialDirectory) const -> std::string
	{
		return ShowFileDialog(filter, false, initialDirectory);
	}

	auto FileDialogs::SelectFolder(const std::string& initialDirectory) const -> std::string
	{
		return ShowFileDialog(nullptr, false, initialDirectory, true);
	}

	auto FileDialogs::SaveFile(const char* filter) const -> std::string
	{
		return ShowFileDialog(filter, true);
	}

}
