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
		static ProcessResult Run(const std::vector<std::string>& arguments, bool captureErrors = true);
	};

}
