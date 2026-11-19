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
		auto operator=(const TemporaryDirectory&) -> TemporaryDirectory& = delete;
		TemporaryDirectory(TemporaryDirectory&&) = delete;
		auto operator=(TemporaryDirectory&&) -> TemporaryDirectory& = delete;

		[[nodiscard]] auto GetPath() const noexcept -> const std::filesystem::path& { return m_Path; }

	private:
		std::filesystem::path m_Path;
	};

}
