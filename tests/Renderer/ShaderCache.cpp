#include "rdpch.h"

#include "Rod/Renderer/ShaderCache.h"

#include "../TemporaryDirectory.h"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <fstream>
#include <vector>

TEST_CASE("Shader cache reads artifacts across instances", "[renderer][shader-cache]")
{
	const Rod::Tests::TemporaryDirectory directory;
	const auto cacheDirectory = directory.GetPath() / "cache";
	constexpr auto words = std::to_array<uint32_t>({0x07230203, 0x10000, 0, 1, 0, 0});

	{
		const Rod::ShaderCache cache(cacheDirectory);
		REQUIRE_FALSE(cache.Load(42));
		REQUIRE(cache.Store(42, words));
	}

	const Rod::ShaderCache cache(cacheDirectory);
	const std::vector<uint32_t> expected(words.begin(), words.end());
	REQUIRE(cache.Load(42) == expected);

	const auto artifact = std::filesystem::directory_iterator(cacheDirectory)->path();
	std::ofstream(artifact, std::ios::binary | std::ios::trunc) << "broken";
	REQUIRE_FALSE(cache.Load(42));
	REQUIRE(cache.Store(42, words));

	REQUIRE(cache.Load(42) == expected);
}