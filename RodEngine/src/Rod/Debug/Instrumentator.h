#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <thread>

namespace Rod {
	struct ProfileResult
	{
		std::string Name;
		long long Start, End;
		size_t ThreadID;
	};

	struct InstrumentationSession
	{
		std::string Name;
	};

	class Instrumentor
	{
	private:
		std::unique_ptr<InstrumentationSession> m_CurrentSession;
		std::ofstream m_OutputStream;
		int m_ProfileCount = 0;
		bool m_Enabled = false;

	public:
		Instrumentor() = default;

		void BeginSession(const std::string& name, const std::filesystem::path& filepath = "results.json")
		{
			if (m_CurrentSession)
				EndSession();

			m_Enabled = true;
			m_OutputStream.open(filepath);
			WriteHeader();
			m_CurrentSession = std::make_unique<InstrumentationSession>(InstrumentationSession{name});
		}

		void EndSession()
		{
			if (!m_CurrentSession)
				return;

			m_Enabled = false;
			WriteFooter();
			m_OutputStream.close();
			m_CurrentSession.reset();
			m_ProfileCount = 0;
		}

		void WriteProfile(const ProfileResult& result)
		{
			if (!m_Enabled || !m_CurrentSession)
				return;

			std::string name = result.Name;
			std::replace(name.begin(), name.end(), '"', '\'');

			if (m_ProfileCount++ > 0)
				m_OutputStream << ",";

			m_OutputStream << "{";
			m_OutputStream << "\"cat\":\"function\",";
			m_OutputStream << "\"dur\":" << (result.End - result.Start) << ',';
			m_OutputStream << "\"name\":\"" << name << "\",";
			m_OutputStream << "\"ph\":\"X\",";
			m_OutputStream << "\"pid\":0,";
			m_OutputStream << "\"tid\":" << result.ThreadID << ",";
			m_OutputStream << "\"ts\":" << result.Start;
			m_OutputStream << "}";

			m_OutputStream.flush();
		}

		void WriteHeader()
		{
			m_OutputStream << "{\"otherData\": {},\"traceEvents\":[";
			m_OutputStream.flush();
		}

		void WriteFooter()
		{
			m_OutputStream << "]}";
			m_OutputStream.flush();
		}

		[[nodiscard]] static auto Get() -> Instrumentor&
		{
			static Instrumentor s_Instance;
			return s_Instance;
		}
	};

	class InstrumentationTimer
	{
	public:
		explicit InstrumentationTimer(const char* name) noexcept : m_Name(name)
		{
			m_StartTimepoint = std::chrono::high_resolution_clock::now();
		}

		~InstrumentationTimer() noexcept
		{
			if (m_Stopped)
				return;

			try
			{
				Stop();
			}
			catch (...) // NOLINT(bugprone-empty-catch)
			{}
		}
		InstrumentationTimer(const InstrumentationTimer&) = delete;
		auto operator=(const InstrumentationTimer&) -> InstrumentationTimer& = delete;
		InstrumentationTimer(InstrumentationTimer&&) = delete;
		auto operator=(InstrumentationTimer&&) -> InstrumentationTimer& = delete;

		void Stop()
		{
			const auto endTimepoint = std::chrono::high_resolution_clock::now();

			const long long start =
			    std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
			const long long end =
			    std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

			const size_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
			Instrumentor::Get().WriteProfile({.Name = m_Name, .Start = start, .End = end, .ThreadID = threadID});

			m_Stopped = true;
		}

	private:
		const char* m_Name;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
		bool m_Stopped = false;
	};

}

#define RD_PROFILE 1
#if RD_PROFILE
	#if defined(_MSC_VER)
		#define RD_PROFILE_FUNC_SIG __FUNCSIG__
	#else
		#define RD_PROFILE_FUNC_SIG __PRETTY_FUNCTION__
	#endif
	#define RD_PROFILE_CONCAT_IMPL(lhs, rhs) lhs##rhs
	#define RD_PROFILE_CONCAT(lhs, rhs) RD_PROFILE_CONCAT_IMPL(lhs, rhs)
	#define RD_PROFILE_BEGIN_SESSION(name, filepath) ::Rod::Instrumentor::Get().BeginSession(name, filepath)
	#define RD_PROFILE_END_SESSION() ::Rod::Instrumentor::Get().EndSession()
	#define RD_PROFILE_SCOPE(name) const ::Rod::InstrumentationTimer RD_PROFILE_CONCAT(timer, __LINE__)(name)
	#define RD_PROFILE_FUNCTION() RD_PROFILE_SCOPE(RD_PROFILE_FUNC_SIG)
#else
	#define RD_PROFILE_BEGIN_SESSION(name, filepath)
	#define RD_PROFILE_END_SESSION()
	#define RD_PROFILE_FUNCTION()
	#define RD_PROFILE_SCOPE(name)
#endif
