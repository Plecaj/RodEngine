#pragma once

#include "Event.h"

#include <sstream>

namespace Rod {

	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(const float x, const float y) noexcept : m_MouseX(x), m_MouseY(y) {}

		[[nodiscard]] float GetX() const noexcept { return m_MouseX; }
		[[nodiscard]] float GetY() const noexcept { return m_MouseY; }

		[[nodiscard]] virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		float m_MouseX, m_MouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(const float xOffset, const float yOffset) noexcept : m_XOffset(xOffset), m_YOffset(yOffset)
		{}

		[[nodiscard]] float GetXOffset() const noexcept { return m_XOffset; }
		[[nodiscard]] float GetYOffset() const noexcept { return m_YOffset; }

		[[nodiscard]] virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << GetXOffset() << ", " << GetYOffset();
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		float m_XOffset, m_YOffset;
	};

	class MouseButtonEvent : public Event
	{
	public:
		[[nodiscard]] int GetMouseButton() const noexcept { return m_Button; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)
	protected:
		explicit MouseButtonEvent(const int button) noexcept : m_Button(button) {}

		int m_Button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		explicit MouseButtonPressedEvent(const int button) noexcept : MouseButtonEvent(button) {}

		[[nodiscard]] virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		explicit MouseButtonReleasedEvent(const int button) noexcept : MouseButtonEvent(button) {}

		[[nodiscard]] virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};

}
