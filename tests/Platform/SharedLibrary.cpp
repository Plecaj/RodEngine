#include "rdpch.h"

#include "Rod/Utils/SharedLibrary.h"

#include "Rod/Utils/FunctionPointerCast.h"

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>

TEST_CASE("Shared libraries load functions and transfer ownership", "[platform][loading]")
{
	REQUIRE_THROWS_AS(Rod::SharedLibrary(std::string(ROD_TEST_LIBRARY_PATH) + ".missing"), std::runtime_error);
	Rod::SharedLibrary library(ROD_TEST_LIBRARY_PATH);
	auto moved = std::move(library);
	// NOLINTNEXTLINE(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
	REQUIRE_FALSE(library.IsLoaded());
	Rod::SharedLibrary replacement(ROD_TEST_LIBRARY_PATH);
	replacement = std::move(moved);
	// NOLINTNEXTLINE(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
	REQUIRE_FALSE(moved.IsLoaded());
	auto* const address = replacement.GetFunction("RodTestValue");
	REQUIRE(address != nullptr);
	const auto function = Rod::FunctionPointerCast<int (*)()>(address);
	REQUIRE(function() == 42);
	REQUIRE(replacement.GetFunction("MissingFunction") == nullptr);
}
