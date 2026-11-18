#include "rdpch.h"

#include "Rod/Renderer/ShaderCacheDatabase.h"

#include "../TemporaryDirectory.h"

#include <catch2/catch_test_macros.hpp>

#include <bit>
#include <fstream>
#include <iterator>

TEST_CASE("Shader cache replacements discard old binary data", "[renderer][shader-cache]")
{
	const Rod::Tests::TemporaryDirectory directory;
	const auto cacheDirectory = directory.GetPath() / "Cached";
	Rod::ShaderCacheDatabase cache(cacheDirectory);
	constexpr auto previous = std::to_array<uint32_t>({4, 3, 2, 1});
	constexpr auto words = std::to_array<uint32_t>({1, 2});
	REQUIRE(cache.CacheShader("shader.glsl", previous));
	REQUIRE(cache.CacheShader("shader.glsl", words));
	std::ifstream file(cacheDirectory / "shader.glsl", std::ios::binary);
	const std::string bytes{std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
	const auto expected = std::bit_cast<std::array<char, sizeof(words)>>(words);
	REQUIRE(std::ranges::equal(bytes, expected));
}

TEST_CASE("Shader cache cleanup retains only valid entries", "[renderer][shader-cache]")
{
	if (!Rod::Log::GetCoreLogger())
		Rod::Log::Init();
	const Rod::Tests::TemporaryDirectory directory;
	const auto cacheDirectory = directory.GetPath() / "Cached";
	Rod::ShaderCacheDatabase original(cacheDirectory);
	constexpr auto words = std::to_array<uint32_t>({1, 2});
	for (const auto* key : {"valid.glsl", "stale.glsl", "missing.glsl"})
		REQUIRE(original.CacheShader(key, words));
	original.Flush();
	std::filesystem::remove(cacheDirectory / "missing.glsl");
	std::ofstream(cacheDirectory / "orphan.glsl") << "old";
	REQUIRE(std::filesystem::exists(cacheDirectory / "orphan.glsl"));

	Rod::ShaderCacheDatabase cache(cacheDirectory);
	cache.AddValidKey("valid.glsl");
	cache.AddValidKey("missing.glsl");
	cache.Flush();
	REQUIRE(cache.Contains("valid.glsl"));
	REQUIRE_FALSE(cache.Contains("stale.glsl"));
	REQUIRE_FALSE(cache.Contains("missing.glsl"));
	REQUIRE(std::filesystem::exists(cacheDirectory / "valid.glsl"));
	REQUIRE_FALSE(std::filesystem::exists(cacheDirectory / "stale.glsl"));
	REQUIRE_FALSE(std::filesystem::exists(cacheDirectory / "orphan.glsl"));
}

TEST_CASE("Failed shader cache writes do not add entries", "[renderer][shader-cache]")
{
	const Rod::Tests::TemporaryDirectory directory;
	Rod::ShaderCacheDatabase cache(directory.GetPath());
	std::filesystem::create_directory(directory.GetPath() / "blocked.glsl");
	constexpr auto words = std::to_array<uint32_t>({1, 2});

	REQUIRE_FALSE(cache.CacheShader("blocked.glsl", words));
	REQUIRE_FALSE(cache.Contains("blocked.glsl"));
}
