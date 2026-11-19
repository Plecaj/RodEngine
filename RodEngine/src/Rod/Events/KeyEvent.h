#pragma once

#include "Event.h"

#include <sstream>

namespace Rod {

	class KeyEvent : public Event
	{
	public:
		[[nodiscard]] constexpr auto GetKeyCode() const noexcept -> int { return m_KeyCode; }

		EVENT_CLASS_CATEGORY(
		    static_cast<unsigned int>(EventCategoryKeyboard) | static_cast<unsigned int>(EventCategoryInput))
	protected:
		constexpr explicit KeyEvent(const int keycode) noexcept : m_KeyCode(keycode) {}

		int m_KeyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		constexpr explicit KeyPressedEvent(const int keycode, bool isRepeat = false) noexcept
		    : KeyEvent(keycode), m_IsRepeat(isRepeat)
		{}

		[[nodiscard]] constexpr auto IsRepeat() const noexcept -> bool { return m_IsRepeat; }

		[[nodiscard]] auto ToString() const -> std::string override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " (repeat = " << m_IsRepeat << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		bool m_IsRepeat;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		constexpr explicit KeyReleasedEvent(const int keycode) noexcept : KeyEvent(keycode) {}

		[[nodiscard]] auto ToString() const -> std::string override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		constexpr explicit KeyTypedEvent(const int keycode) noexcept : KeyEvent(keycode) {}

		[[nodiscard]] auto ToString() const -> std::string override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)
	};
}
