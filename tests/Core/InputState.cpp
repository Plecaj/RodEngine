#include "rdpch.h"

#include "Rod/Core/InputState.h"

#include "Rod/Core/KeyCodes.h"
#include "Rod/Core/MouseButtonCodes.h"
#include "Rod/Events/Event.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Input keeps the final state of ordered key events", "[core][input]")
{
	Rod::InputState input;
	const std::array<Rod::Event, 4> events{
	    Rod::KeyChanged{.Key = Rod::Key::LeftControl, .Action = Rod::KeyAction::Press, .Mods = {.Control = true}},
	    Rod::KeyChanged{.Key = Rod::Key::S, .Action = Rod::KeyAction::Press, .Mods = {.Control = true}},
	    Rod::KeyChanged{.Key = Rod::Key::S, .Action = Rod::KeyAction::Release, .Mods = {.Control = true}},
	    Rod::KeyChanged{.Key = Rod::Key::LeftControl, .Action = Rod::KeyAction::Release, .Mods = {}},
	};
	input.Update(events, {});

	REQUIRE_FALSE(input.IsKeyDown(Rod::Key::S));
	REQUIRE_FALSE(input.IsKeyDown(Rod::Key::LeftControl));
	REQUIRE(std::get<Rod::KeyChanged>(events[1]).Mods.Control);
	REQUIRE(std::get<Rod::KeyChanged>(events[1]).Action == Rod::KeyAction::Press);
}

TEST_CASE("Input retains holds across frames and discards them on focus loss", "[core][input]")
{
	Rod::InputState input({1.0f, 2.0f});
	const std::array<Rod::Event, 2> presses{
	    Rod::KeyChanged{.Key = Rod::Key::W, .Action = Rod::KeyAction::Press, .Mods = {}},
	    Rod::MouseButtonChanged{.Button = Rod::Mouse::ButtonLeft, .Down = true, .Mods = {}, .Position = {}},
	};
	input.Update(presses, {3.0f, 4.0f});
	REQUIRE(input.IsKeyDown(Rod::Key::W));
	REQUIRE(input.IsMouseDown(Rod::Mouse::ButtonLeft));
	REQUIRE(input.GetCursorDelta() == glm::vec2(2.0f, 2.0f));

	input.Update({}, {4.0f, 4.0f});
	REQUIRE(input.IsKeyDown(Rod::Key::W));
	REQUIRE(input.IsMouseDown(Rod::Mouse::ButtonLeft));

	const std::array<Rod::Event, 1> focusLost{Rod::FocusChanged{false}};
	input.Update(focusLost, {0.0f, 0.0f});
	REQUIRE_FALSE(input.IsKeyDown(Rod::Key::W));
	REQUIRE_FALSE(input.IsMouseDown(Rod::Mouse::ButtonLeft));
	REQUIRE(input.GetCursorDelta() == glm::vec2(0.0f));

	const std::array<Rod::Event, 2> focusRestored{
	    Rod::FocusChanged{true},
	    Rod::KeyChanged{.Key = Rod::Key::W, .Action = Rod::KeyAction::Repeat, .Mods = {}},
	};
	input.Update(focusRestored, {2.0f, 1.0f});
	REQUIRE_FALSE(input.IsKeyDown(Rod::Key::W));
	REQUIRE(input.GetCursorDelta() == glm::vec2(0.0f));
	REQUIRE(input.GetCursor() == glm::vec2(2.0f, 1.0f));
}
