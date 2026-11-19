#pragma once

#include <csignal>
#include <memory>
#include <utility>

#if !defined(RD_PLATFORM_WINDOWS) && !defined(RD_PLATFORM_LINUX)
	#error Rod only supports Windows and Linux.
#endif

#ifdef RD_PLATFORM_WINDOWS
	#define RD_DEBUGBREAK() __debugbreak()
	#define RD_CDECL __cdecl
#else
	#define RD_DEBUGBREAK() std::raise(SIGTRAP)
	#define RD_CDECL
#endif

#ifdef RD_DEBUG
	#define RD_ENABLE_ASSERTS
#endif

#ifdef RD_ENABLE_ASSERTS
	#define RD_ASSERT(x, ...) \
		do \
		{ \
			if (!(x)) \
			{ \
				RD_ERROR("Assertion Failed: {0}", __VA_ARGS__); \
				RD_DEBUGBREAK(); \
			} \
		} while (false);
	#define RD_CORE_ASSERT(x, ...) \
		do \
		{ \
			if (!(x)) \
			{ \
				RD_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); \
				RD_DEBUGBREAK(); \
			} \
		} while (false);
#else
	#define RD_ASSERT(x, ...) \
		do \
		{ \
			(void)sizeof(x); \
		} while (false);
	#define RD_CORE_ASSERT(x, ...) \
		do \
		{ \
			(void)sizeof(x); \
		} while (false);
#endif

namespace Rod {
	template <typename T>
	using ObserverPtr = T*;

	template <typename T, typename Deleter = std::default_delete<T>>
	using Scope = std::unique_ptr<T, Deleter>;

	template <typename T, typename... Args>
	[[nodiscard]] constexpr auto CreateScope(Args&&... args) -> Scope<T>
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template <typename T>
	using Ref = std::shared_ptr<T>;

	template <typename T, typename... Args>
	[[nodiscard]] auto CreateRef(Args&&... args) -> Ref<T>
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}
