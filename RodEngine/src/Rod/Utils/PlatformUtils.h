#pragma once

#include <filesystem>
#include <string>

namespace Rod {

	class Platform
	{
	public:
		[[nodiscard]] static float GetTime();
		[[nodiscard]] static std::filesystem::path GetExecutablePath();
	};

	class FileDialogs
	{
	public:
		[[nodiscard]] static std::string OpenFile(const char* filter, const std::string& initialDirectory = {});
		[[nodiscard]] static std::string SelectFolder(const std::string& initialDirectory = {});
		[[nodiscard]] static std::string SaveFile(const char* filter);
	};

}
