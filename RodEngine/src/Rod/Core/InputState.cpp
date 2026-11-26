#include "rdpch.h"

#include "InputState.h"

namespace Rod {

	void InputState::Update(std::span<const Event> events, glm::vec2 cursor) noexcept
	{
		bool focusChanged = false;
		for (const Event& event : events)
		{
			if (const auto* focus = std::get_if<FocusChanged>(&event))
			{
				m_Focused = focus->Focused;
				focusChanged = true;
				if (!m_Focused)
				{
					m_Keys.reset();
					m_Buttons.reset();
				}
			}
			else if (const auto* key = std::get_if<KeyChanged>(&event))
			{
				if (key->Key >= Key::Space && key->Key <= Key::Menu && key->Action != KeyAction::Repeat)
					m_Keys[static_cast<size_t>(key->Key)] = m_Focused && key->Action == KeyAction::Press;
			}
			else if (
			    const auto* mouse = std::get_if<MouseButtonChanged>(&event);
			    mouse && mouse->Button >= Mouse::Button0 && mouse->Button <= Mouse::ButtonLast)
			{
				m_Buttons[static_cast<size_t>(mouse->Button)] = m_Focused && mouse->Down;
			}
		}

		m_CursorDelta = m_Focused && !focusChanged ? cursor - m_Cursor : glm::vec2{};
		m_Cursor = cursor;
	}

	auto InputState::IsKeyDown(int key) const noexcept -> bool
	{
		return key >= Key::Space && key <= Key::Menu && m_Keys[static_cast<size_t>(key)];
	}

	auto InputState::IsMouseDown(int button) const noexcept -> bool
	{
		return button >= Mouse::Button0 && button <= Mouse::ButtonLast && m_Buttons[static_cast<size_t>(button)];
	}

}
