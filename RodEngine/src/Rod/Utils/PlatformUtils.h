#pragma once

#include <string>

namespace Rod {

	class Platform
	{
	public:
		static float GetTime();
	};

	class FileDialogs
	{
	public:
		static std::string OpenFile(const char* filter);
		static std::string SaveFile(const char* filter);
	};

}
