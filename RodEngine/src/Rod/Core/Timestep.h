#pragma once

namespace Rod {

	class Timestep
	{
	public:
		constexpr Timestep() noexcept = default;
		constexpr explicit Timestep(float time) noexcept : m_Time(time) {}

		[[nodiscard]] constexpr explicit operator float() const noexcept { return m_Time; };

		[[nodiscard]] constexpr float GetSeconds() const noexcept { return m_Time; };
		[[nodiscard]] constexpr float GetMilliseconds() const noexcept { return m_Time * MILLISECONDS_PER_SECOND; };

	private:
		static constexpr float MILLISECONDS_PER_SECOND = 1000.0f;
		float m_Time = 0.0f;
	};

}
