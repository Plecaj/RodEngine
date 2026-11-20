#pragma once

namespace Rod {

	template <typename... Flags>
	[[nodiscard]] constexpr auto CombineImGuiFlags(Flags... flags) noexcept -> int
	{
		return static_cast<int>((0U | ... | static_cast<unsigned>(flags)));
	}

	[[nodiscard]] constexpr auto WithoutImGuiFlags(int flags, int mask) noexcept -> int
	{
		return static_cast<int>(static_cast<unsigned>(flags) & ~static_cast<unsigned>(mask));
	}

	[[nodiscard]] constexpr auto HasAnyImGuiFlag(int flags, int mask) noexcept -> bool
	{
		return (static_cast<unsigned>(flags) & static_cast<unsigned>(mask)) != 0;
	}

}
