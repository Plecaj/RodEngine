#pragma once

#include <filesystem>
#include <string>

namespace Rod {

	class Platform
	{
	public:
		static float GetTime();
		[[nodiscard]] static std::filesystem::path GetExecutablePath();
	};

	class FileDialogs
	{
	public:
		static std::string OpenFile(const char* filter, const std::string& initialDirectory = {});
		static std::string SelectFolder(const std::string& initialDirectory = {});
		static std::string SaveFile(const char* filter);
	};

}
