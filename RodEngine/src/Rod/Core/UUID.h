#pragma once

#include <cstdint>

namespace Rod {

	class UUID
	{
	public:
		UUID();
		explicit UUID(uint64_t uuid) noexcept;

		[[nodiscard]] explicit operator uint64_t() const noexcept { return m_UUID; }
		[[nodiscard]] bool operator==(const UUID&) const noexcept = default;

	private:
		uint64_t m_UUID;
	};
}
