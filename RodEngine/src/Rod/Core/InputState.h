#pragma once

#include "Rod/Core/KeyCodes.h"
#include "Rod/Core/MouseButtonCodes.h"
#include "Rod/Events/Event.h"

#include <bitset>
#include <span>

namespace Rod {

	class InputState
	{
	public:
		explicit InputState(glm::vec2 cursor = {}) noexcept : m_Cursor(cursor) {}

		void Update(std::span<const Event> events, glm::vec2 cursor) noexcept;
		[[nodiscard]] auto IsKeyDown(int key) const noexcept -> bool;
		[[nodiscard]] auto IsMouseDown(int button) const noexcept -> bool;
		[[nodiscard]] auto GetCursor() const noexcept -> glm::vec2 { return m_Cursor; }
		[[nodiscard]] auto GetCursorDelta() const noexcept -> glm::vec2 { return m_CursorDelta; }

	private:
		std::bitset<Key::Menu + 1> m_Keys;
		std::bitset<Mouse::ButtonLast + 1> m_Buttons;
		glm::vec2 m_Cursor{};
		glm::vec2 m_CursorDelta{};
		bool m_Focused = true;
	};

}
