#pragma once

#include "Rod/Core/Core.h"

#include <cstdint>
#include <ostream>
#include <string>

namespace Rod {

	enum class EventType : std::uint8_t
	{
		None = 0,
		WindowClose,
		WindowResize,
		WindowFocus,
		WindowLostFocus,
		WindowMoved,
		AppTick,
		AppUpdate,
		AppRender,
		KeyPressed,
		KeyReleased,
		KeyTyped,
		MouseButtonPressed,
		MouseButtonReleased,
		MouseMoved,
		MouseScrolled
	};

	enum EventCategory : std::uint8_t
	{
		None = 0,
		EventCategoryApplication = BIT(0),
		EventCategoryInput = BIT(1),
		EventCategoryKeyboard = BIT(2),
		EventCategoryMouse = BIT(3),
		EventCategoryMouseButton = BIT(4)
	};

#define EVENT_CLASS_TYPE(type) \
	[[nodiscard]] static EventType GetStaticType() noexcept \
	{ \
		return EventType::type; \
	} \
	[[nodiscard]] virtual EventType GetEventType() const noexcept override \
	{ \
		return GetStaticType(); \
	} \
	[[nodiscard]] virtual const char* GetName() const noexcept override \
	{ \
		return #type; \
	}

#define EVENT_CLASS_CATEGORY(category) \
	[[nodiscard]] virtual int GetCategoryFlags() const noexcept override \
	{ \
		return category; \
	}

	class Event
	{
	public:
		Event() = default;
		virtual ~Event() = default;
		Event(const Event&) = delete;
		Event& operator=(const Event&) = delete;
		Event(Event&&) = delete;
		Event& operator=(Event&&) = delete;

		bool Handled = false;

		[[nodiscard]] virtual EventType GetEventType() const noexcept = 0;
		[[nodiscard]] virtual const char* GetName() const noexcept = 0;
		[[nodiscard]] virtual int GetCategoryFlags() const noexcept = 0;
		[[nodiscard]] virtual std::string ToString() const { return GetName(); }

		[[nodiscard]] bool IsInCategory(EventCategory category) const noexcept
		{
			return (GetCategoryFlags() & category) != 0;
		}
	};

	class EventDispatcher
	{
	public:
		explicit EventDispatcher(Event& event) noexcept : m_Event(event) {}

		template <typename T, typename F>
		bool Dispatch(const F& func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.Handled |= func(static_cast<T&>(m_Event));
				return true;
			}
			return false;
		}

	private:
		Event& m_Event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}

}
