#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <vector>

namespace Rod {

	class ShaderCache
	{
	public:
		explicit ShaderCache(std::filesystem::path directory);

		[[nodiscard]] auto Load(uint64_t key) const -> std::optional<std::vector<uint32_t>>;
		[[nodiscard]] auto Store(uint64_t key, std::span<const uint32_t> words) const -> bool;

	private:
		[[nodiscard]] auto PathFor(uint64_t key) const -> std::filesystem::path;

		std::filesystem::path m_Directory;
	};

}
