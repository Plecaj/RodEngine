#pragma once

#include "Rod/Core/Core.h"

#include <cstdint>
#include <ostream>
#include <string>
#include <utility>

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
	[[nodiscard]] static constexpr EventType GetStaticType() noexcept \
	{ \
		return EventType::type; \
	} \
	[[nodiscard]] virtual constexpr EventType GetEventType() const noexcept override \
	{ \
		return GetStaticType(); \
	} \
	[[nodiscard]] virtual constexpr const char* GetName() const noexcept override \
	{ \
		return #type; \
	}

#define EVENT_CLASS_CATEGORY(category) \
	[[nodiscard]] virtual constexpr int GetCategoryFlags() const noexcept override \
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

		[[nodiscard]] virtual constexpr EventType GetEventType() const noexcept = 0;
		[[nodiscard]] virtual constexpr const char* GetName() const noexcept = 0;
		[[nodiscard]] virtual constexpr int GetCategoryFlags() const noexcept = 0;
		[[nodiscard]] virtual std::string ToString() const { return GetName(); }

		[[nodiscard]] constexpr bool IsInCategory(EventCategory category) const noexcept
		{
			return (GetCategoryFlags() & category) != 0;
		}
	};

	template <typename T, typename F>
	constexpr void HandleEvent(Event& event, F&& callback)
	{
		if (event.GetEventType() != T::GetStaticType())
			return;

		if (std::forward<F>(callback)(static_cast<T&>(event)))
			event.Handled = true;
	}

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}

}
