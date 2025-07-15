#pragma once

#include <memory>

#ifdef RD_PLATFORM_WINDOWS
#if RD_DYNAMIC_LINK
	#ifdef RD_BUILD_DLL
		#define ROD_API __declspec(dllexport)
	#else
		#define ROD_API __declspec(dllimport)
	#endif
#else
	#define ROD_API 
#endif
#else
	#error Rod only supports Windows.
#endif

#ifdef RD_DEBUG
	#define RD_ENABLE_ASSERTS
#endif

#ifdef RD_ENABLE_ASSERTS
	#define RD_ASSERT(x, ...) {if(!(x)) {RD_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); }}
	#define RD_CORE_ASSERT(x, ...) {if(!(x)) {RD_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); }}
#else
	#define RD_ASSERT(x, ...)
	#define RD_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define RD_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace Rod
{
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename... Args>
	inline static constexpr Scope<T> CreateScope(Args&&... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename... Args>
	inline static constexpr Ref<T> CreateRef(Args&&... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}