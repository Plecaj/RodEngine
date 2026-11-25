#include "rdpch.h"

#include "ShaderCache.h"

#include <fstream>
#include <limits>
#include <random>
#include <system_error>

namespace Rod {
	namespace {
		constexpr uint32_t g_SpirvMagic = 0x07230203;
		constexpr size_t g_SpirvHeaderWords = 5;
	}

	ShaderCache::ShaderCache(std::filesystem::path directory) : m_Directory(std::move(directory))
	{}

	auto ShaderCache::PathFor(uint64_t key) const -> std::filesystem::path
	{
		return m_Directory / ("rod-" + std::to_string(key) + ".spv");
	}

	auto ShaderCache::Load(uint64_t key) const -> std::optional<std::vector<uint32_t>>
	{
		std::ifstream input(PathFor(key), std::ios::binary | std::ios::ate);
		if (!input)
			return std::nullopt;

		const auto bytes = static_cast<std::streamoff>(input.tellg());
		constexpr auto wordSize = static_cast<std::streamoff>(sizeof(uint32_t));
		if (bytes < static_cast<std::streamoff>(g_SpirvHeaderWords) * wordSize || bytes % wordSize != 0 ||
		    !std::in_range<std::streamsize>(bytes) || !std::in_range<size_t>(bytes))
			return std::nullopt;

		std::vector<uint32_t> words(static_cast<size_t>(bytes / wordSize));
		input.seekg(0);
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		input.read(reinterpret_cast<char*>(words.data()), static_cast<std::streamsize>(bytes));
		if (!input || words.front() != g_SpirvMagic)
			return std::nullopt;
		return words;
	}

	auto ShaderCache::Store(uint64_t key, std::span<const uint32_t> words) const -> bool
	{
		const auto maxBytes = std::numeric_limits<std::streamsize>::max();
		if (words.size() < g_SpirvHeaderWords || words.front() != g_SpirvMagic ||
		    std::cmp_greater(words.size(), maxBytes / sizeof(uint32_t)))
			return false;

		std::error_code error;
		std::filesystem::create_directories(m_Directory, error);
		if (error)
			return false;

		const auto path = PathFor(key);
		std::random_device random;
		auto temporary = path;
		temporary += "." + std::to_string(random()) + "-" + std::to_string(random()) + ".tmp";
		{
			std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
			if (!output)
				return false;

			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			output.write(reinterpret_cast<const char*>(words.data()), static_cast<std::streamsize>(words.size_bytes()));
			output.close();
			if (output)
			{
				std::filesystem::rename(temporary, path, error);
				if (!error)
					return true;
			}
		}

		std::filesystem::remove(temporary, error);
		return false;
	}

}
