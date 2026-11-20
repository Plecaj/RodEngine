#pragma once

#include <concepts>
#include <type_traits>

namespace Rod {

	template <typename T>
	concept FunctionPointer = std::is_pointer_v<T> && std::is_function_v<std::remove_pointer_t<T>>;

	template <typename To, typename From>
	    requires(
	        (FunctionPointer<To> && std::same_as<From, void*>) || (std::same_as<To, void*> && FunctionPointer<From>))
	[[nodiscard]] auto FunctionPointerCast(From pointer) noexcept -> To
	{
		return reinterpret_cast<To>(pointer); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
	}

}
