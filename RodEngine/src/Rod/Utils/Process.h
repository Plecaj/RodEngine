#pragma once

#include <string>
#include <vector>

namespace Rod {

	struct ProcessResult
	{
		int ExitCode = -1;
		std::string Output;
	};

	class Process
	{
	public:
		[[nodiscard]] static auto Start(const std::vector<std::string>& arguments) -> ProcessResult;
		[[nodiscard]] static auto Run(const std::vector<std::string>& arguments, bool captureErrors = true)
		    -> ProcessResult;
	};

}
