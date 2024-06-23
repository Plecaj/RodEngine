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

#define BIT(x) (1 << (x))