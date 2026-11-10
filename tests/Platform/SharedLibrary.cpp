#include "rdpch.h"

#include "Rod/Utils/SharedLibrary.h"

#include <catch2/catch_test_macros.hpp>

#include <bit>
#include <stdexcept>

TEST_CASE("Shared libraries load functions and transfer ownership", "[platform][loading]")
{
	REQUIRE_THROWS_AS(Rod::SharedLibrary(std::string(ROD_TEST_LIBRARY_PATH) + ".missing"), std::runtime_error);
	Rod::SharedLibrary library(ROD_TEST_LIBRARY_PATH);
	auto moved = std::move(library);
	REQUIRE_FALSE(library.IsLoaded());
	Rod::SharedLibrary replacement(ROD_TEST_LIBRARY_PATH);
	replacement = std::move(moved);
	REQUIRE_FALSE(moved.IsLoaded());
	const auto function = std::bit_cast<int (*)()>(replacement.GetFunction("RodTestValue"));
	REQUIRE(function != nullptr);
	REQUIRE(function() == 42);
	REQUIRE(replacement.GetFunction("MissingFunction") == nullptr);
}
