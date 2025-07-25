#pragma once

#include <string>

namespace Rod {

	class FileDialogs
	{
	public:
		// Returns empty string if cancelled
		static std::string OpenFile(const char* filter);

		// Returns empty string if cancelled
		static std::string SaveFile(const char* filter);
	};

}