#pragma once

#ifdef RD_PLATFORM_WINDOWS
	#ifdef RD_BUILD_DLL
		#define ROD_API __declspec(dllexport)
	#else
		#define ROD_API __declspec(dllimport)
	#endif
#else
	#error Rod only supports Windows.
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