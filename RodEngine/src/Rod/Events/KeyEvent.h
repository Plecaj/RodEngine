#pragma once

#include "Event.h"

#include <sstream>

namespace Rod {

	class KeyEvent : public Event
	{
	public:
		[[nodiscard]] int GetKeyCode() const noexcept { return m_KeyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
		explicit KeyEvent(const int keycode) noexcept : m_KeyCode(keycode) {}

		int m_KeyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		explicit KeyPressedEvent(const int keycode, bool isRepeat = false) noexcept
		    : KeyEvent(keycode), m_IsRepeat(isRepeat)
		{}

		[[nodiscard]] bool IsRepeat() const noexcept { return m_IsRepeat; }

		[[nodiscard]] virtual std::string ToString() const override
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
		explicit KeyReleasedEvent(const int keycode) noexcept : KeyEvent(keycode) {}

		[[nodiscard]] virtual std::string ToString() const override
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
		explicit KeyTypedEvent(const int keycode) noexcept : KeyEvent(keycode) {}

		[[nodiscard]] virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)
	};
}
