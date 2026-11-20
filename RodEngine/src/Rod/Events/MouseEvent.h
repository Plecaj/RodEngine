#pragma once

#include "Event.h"

#include <sstream>

namespace Rod {

	class MouseMovedEvent : public Event
	{
	public:
		constexpr MouseMovedEvent(const float x, const float y) noexcept : m_MouseX(x), m_MouseY(y) {}

		[[nodiscard]] constexpr auto GetX() const noexcept -> float { return m_MouseX; }
		[[nodiscard]] constexpr auto GetY() const noexcept -> float { return m_MouseY; }

		[[nodiscard]] auto ToString() const -> std::string override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(
		    static_cast<unsigned int>(EventCategoryMouse) | static_cast<unsigned int>(EventCategoryInput))
	private:
		float m_MouseX, m_MouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		constexpr MouseScrolledEvent(const float xOffset, const float yOffset) noexcept
		    : m_XOffset(xOffset), m_YOffset(yOffset)
		{}

		[[nodiscard]] constexpr auto GetXOffset() const noexcept -> float { return m_XOffset; }
		[[nodiscard]] constexpr auto GetYOffset() const noexcept -> float { return m_YOffset; }

		[[nodiscard]] auto ToString() const -> std::string override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << GetXOffset() << ", " << GetYOffset();
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(
		    static_cast<unsigned int>(EventCategoryMouse) | static_cast<unsigned int>(EventCategoryInput))
	private:
		float m_XOffset, m_YOffset;
	};

	class MouseButtonEvent : public Event
	{
	public:
		[[nodiscard]] constexpr auto GetMouseButton() const noexcept -> int { return m_Button; }

		EVENT_CLASS_CATEGORY(
		    static_cast<unsigned int>(EventCategoryMouse) | static_cast<unsigned int>(EventCategoryInput) |
		    static_cast<unsigned int>(EventCategoryMouseButton))
	protected:
		constexpr explicit MouseButtonEvent(const int button) noexcept : m_Button(button) {}

		int m_Button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		constexpr explicit MouseButtonPressedEvent(const int button) noexcept : MouseButtonEvent(button) {}

		[[nodiscard]] auto ToString() const -> std::string override
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
		constexpr explicit MouseButtonReleasedEvent(const int button) noexcept : MouseButtonEvent(button) {}

		[[nodiscard]] auto ToString() const -> std::string override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};

}
