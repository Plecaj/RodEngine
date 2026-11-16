#pragma once

#include "Rod/Core/UUID.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <system_error>

namespace Rod::Tests {

	class TemporaryDirectory
	{
	public:
		TemporaryDirectory()
		{
			do
			{
				m_Path = std::filesystem::temp_directory_path() /
				         ("rod-test-" + std::to_string(static_cast<uint64_t>(UUID{})));
			} while (!std::filesystem::create_directory(m_Path));
		}
		~TemporaryDirectory()
		{
			std::error_code error;
			std::filesystem::remove_all(m_Path, error);
		}
		TemporaryDirectory(const TemporaryDirectory&) = delete;
		TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;
		TemporaryDirectory(TemporaryDirectory&&) = delete;
		TemporaryDirectory& operator=(TemporaryDirectory&&) = delete;

		[[nodiscard]] const std::filesystem::path& GetPath() const noexcept { return m_Path; }

	private:
		std::filesystem::path m_Path;
	};

}
