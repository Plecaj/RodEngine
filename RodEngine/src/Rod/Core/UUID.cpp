#include "rdpch.h"

#include "UUID.h"

#include <random>

namespace Rod {

	namespace {
		auto GenerateUUID() -> uint64_t
		{
			static std::mt19937_64 s_Engine{std::random_device{}()};
			static std::uniform_int_distribution<uint64_t> s_Distribution;
			return s_Distribution(s_Engine);
		}
	}

	UUID::UUID() : m_UUID(GenerateUUID())
	{}
}
