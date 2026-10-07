#include "rdpch.h"
#include "Rod/Utils/PlatformUtils.h"
#include "Rod/Utils/Process.h"

namespace Rod {

	static std::string ShowFileDialog(const char* filter, bool save)
	{
		std::vector<std::string> arguments = { "zenity", "--file-selection" };
		if (save)
		{
			arguments.emplace_back("--save");
			arguments.emplace_back("--confirm-overwrite");
		}

		if (filter && *filter)
		{
			std::string description = filter;
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

	std::string FileDialogs::OpenFile(const char* filter)
	{
		return ShowFileDialog(filter, false);
	}

	std::string FileDialogs::SaveFile(const char* filter)
	{
		return ShowFileDialog(filter, true);
	}

}
