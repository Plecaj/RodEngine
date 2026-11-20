#pragma once

#include <cstdint>

namespace Rod {

	class UUID
	{
	public:
		UUID();
		constexpr explicit UUID(uint64_t uuid) noexcept : m_UUID(uuid) {}

		[[nodiscard]] constexpr explicit operator uint64_t() const noexcept { return m_UUID; }
		[[nodiscard]] constexpr auto operator==(const UUID&) const noexcept -> bool = default;

	private:
		uint64_t m_UUID;
	};
}
