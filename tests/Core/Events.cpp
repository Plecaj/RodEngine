#include "rdpch.h"

#include "Rod/Events/ApplicationEvent.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Events dispatch only matching types", "[core][events]")
{
	Rod::WindowCloseEvent event;
	bool called = false;

	Rod::HandleEvent<Rod::WindowResizeEvent>(event, [&](Rod::WindowResizeEvent&) {
		called = true;
		return true;
	});
	REQUIRE_FALSE(called);

	Rod::HandleEvent<Rod::WindowCloseEvent>(event, [&](const Rod::WindowCloseEvent&) {
		called = true;
		return true;
	});
	REQUIRE(called);
}

TEST_CASE("Event handling preserves handled state", "[core][events]")
{
	Rod::WindowCloseEvent event;
	Rod::HandleEvent<Rod::WindowCloseEvent>(event, [](const auto&) { return false; });
	REQUIRE_FALSE(event.Handled);

	Rod::HandleEvent<Rod::WindowCloseEvent>(event, [](const auto&) { return true; });
	REQUIRE(event.Handled);

	Rod::HandleEvent<Rod::WindowCloseEvent>(event, [](const auto&) { return false; });
	REQUIRE(event.Handled);
}
