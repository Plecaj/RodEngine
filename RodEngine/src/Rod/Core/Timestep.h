#pragma once

namespace Rod {

	class Timestep
	{
	public:
		constexpr Timestep() noexcept = default;
		constexpr explicit Timestep(float time) noexcept : m_Time(time) {}

		[[nodiscard]] constexpr explicit operator float() const noexcept { return m_Time; };

		[[nodiscard]] constexpr auto GetSeconds() const noexcept -> float { return m_Time; };
		[[nodiscard]] constexpr auto GetMilliseconds() const noexcept -> float
		{
			return m_Time * s_MillisecondsPerSecond;
		};

	private:
		static constexpr float s_MillisecondsPerSecond = 1000.0f;
		float m_Time = 0.0f;
	};

}
