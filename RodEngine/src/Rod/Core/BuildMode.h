#pragma once

#include <cstdint>

namespace Rod {

	class BuildMode
	{
	public:
		enum class Mode : std::uint8_t
		{
			Editor,
			Sandbox,
		};

		[[nodiscard]] static Mode& GetMode() noexcept
		{
			static Mode s_Mode = Mode::Sandbox;
			return s_Mode;
		}

		static void SetMode(Mode mode) noexcept { GetMode() = mode; }
	};

}
