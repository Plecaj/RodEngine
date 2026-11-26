#pragma once

#include <filesystem>
#include <string>

namespace Rod {

	class Window;

	class Platform
	{
	public:
		[[nodiscard]] static auto GetExecutablePath() -> std::filesystem::path;
	};

	class FileDialogs
	{
	public:
		FileDialogs() noexcept = default;
		explicit FileDialogs(const Window& owner) noexcept : m_Owner(&owner) {}

		[[nodiscard]] auto OpenFile(const char* filter, const std::string& initialDirectory = {}) const -> std::string;
		[[nodiscard]] auto SelectFolder(const std::string& initialDirectory = {}) const -> std::string;
		[[nodiscard]] auto SaveFile(const char* filter) const -> std::string;

	private:
		[[maybe_unused]] const Window* m_Owner = nullptr;
	};

}
