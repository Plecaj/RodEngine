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
		EventCategoryApplication = 1U << 0U,
		EventCategoryInput = 1U << 1U,
		EventCategoryKeyboard = 1U << 2U,
		EventCategoryMouse = 1U << 3U,
		EventCategoryMouseButton = 1U << 4U
	};

#define EVENT_CLASS_TYPE(type) \
	[[nodiscard]] static constexpr auto GetStaticType() noexcept -> EventType \
	{ \
		return EventType::type; \
	} \
	[[nodiscard]] virtual constexpr auto GetEventType() const noexcept -> EventType override \
	{ \
		return GetStaticType(); \
	} \
	[[nodiscard]] virtual constexpr auto GetName() const noexcept -> const char* override \
	{ \
		return #type; \
	}

#define EVENT_CLASS_CATEGORY(category) \
	[[nodiscard]] virtual constexpr auto GetCategoryFlags() const noexcept -> int override \
	{ \
		return static_cast<int>(category); \
	}

	class Event
	{
	public:
		Event() = default;
		virtual ~Event() = default;
		Event(const Event&) = delete;
		auto operator=(const Event&) -> Event& = delete;
		Event(Event&&) = delete;
		auto operator=(Event&&) -> Event& = delete;

		bool Handled = false;

		[[nodiscard]] virtual constexpr auto GetEventType() const noexcept -> EventType = 0;
		[[nodiscard]] virtual constexpr auto GetName() const noexcept -> const char* = 0;
		[[nodiscard]] virtual constexpr auto GetCategoryFlags() const noexcept -> int = 0;
		[[nodiscard]] virtual auto ToString() const -> std::string { return GetName(); }

		[[nodiscard]] constexpr auto IsInCategory(EventCategory category) const noexcept -> bool
		{
			return (static_cast<unsigned int>(GetCategoryFlags()) & static_cast<unsigned int>(category)) != 0U;
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

	inline auto operator<<(std::ostream& os, const Event& e) -> std::ostream&
	{
		return os << e.ToString();
	}

}
