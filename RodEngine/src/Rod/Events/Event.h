#pragma once

#include <glm/vec2.hpp>

#include <cstdint>
#include <variant>

namespace Rod {

	struct CloseRequested
	{};

	struct FramebufferResized
	{
		glm::uvec2 Size;
	};

	struct FocusChanged
	{
		bool Focused;
	};

	enum class KeyAction : uint8_t
	{
		Press,
		Release,
		Repeat
	};

	struct Modifiers
	{
		bool Control = false;
		bool Shift = false;
		bool Alt = false;
	};

	struct KeyChanged
	{
		int Key = -1;
		KeyAction Action = KeyAction::Release;
		Modifiers Mods;
	};

	struct MouseButtonChanged
	{
		int Button;
		bool Down;
		Modifiers Mods;
		glm::vec2 Position;
	};

	struct Scrolled
	{
		glm::vec2 Offset;
		glm::vec2 Position;
	};

	using Event =
	    std::variant<CloseRequested, FramebufferResized, FocusChanged, KeyChanged, MouseButtonChanged, Scrolled>;

}
