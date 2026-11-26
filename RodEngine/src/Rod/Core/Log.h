#pragma once

#include "Rod/Core/Core.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"

namespace Rod {

	class Log
	{
	public:
		static void Init();

		[[nodiscard]] static auto Core() noexcept -> spdlog::logger& { return *s_CoreLogger; }
		[[nodiscard]] static auto Client() noexcept -> spdlog::logger& { return *s_ClientLogger; }
		[[nodiscard]] static auto IsInitialized() noexcept -> bool { return s_CoreLogger && s_ClientLogger; }

	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
	};

}
