#pragma once

#include <filesystem>
#include <string>

namespace Rod {

	class Window;

	class Platform
	{
	public:
		[[nodiscard]] static float GetTime();
		[[nodiscard]] static std::filesystem::path GetExecutablePath();
	};

	class FileDialogs
	{
	public:
		FileDialogs() noexcept = default;
		explicit FileDialogs(const Window& owner) noexcept : m_Owner(&owner) {}

		[[nodiscard]] std::string OpenFile(const char* filter, const std::string& initialDirectory = {}) const;
		[[nodiscard]] std::string SelectFolder(const std::string& initialDirectory = {}) const;
		[[nodiscard]] std::string SaveFile(const char* filter) const;

	private:
		[[maybe_unused]] const Window* m_Owner = nullptr;
	};

}
