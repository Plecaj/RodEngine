#pragma once

#include "spdlog/spdlog.h"

#include "spdlog/fmt/ostr.h"

namespace Rod {

	class Log
	{
	public:
		static void Init();

		inline static Ref<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static Ref<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
	};

}

// Core log macros
#define RD_CORE_ERROR(...)     ::Rod::Log::GetCoreLogger()->error(__VA_ARGS__)
#define RD_CORE_WARN(...)      ::Rod::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define RD_CORE_INFO(...)      ::Rod::Log::GetCoreLogger()->info(__VA_ARGS__)
#define RD_CORE_TRACE(...)     ::Rod::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define RD_CORE_FATAL(...)     ::Rod::Log::GetCoreLogger()->critical(__VA_ARGS__)

//Client log macros
#define RD_ERROR(...)            ::Rod::Log::GetClientLogger()->error(__VA_ARGS__)
#define RD_WARN(...)             ::Rod::Log::GetClientLogger()->warn(__VA_ARGS__)
#define RD_INFO(...)	         ::Rod::Log::GetClientLogger()->info(__VA_ARGS__)
#define RD_TRACE(...)            ::Rod::Log::GetClientLogger()->trace(__VA_ARGS__)
#define RD_FATAL(...)            ::Rod::Log::GetClientLogger()->critical(__VA_ARGS__)
