#include "rdpch.h"

#include "UUID.h"

#include <random>

namespace Rod {

	static std::random_device g_RandomDevice;
	static std::mt19937_64 g_Engine(g_RandomDevice());
	static std::uniform_int_distribution<uint64_t> g_UniformDistribution;

	UUID::UUID() : m_UUID(g_UniformDistribution(g_Engine))
	{}

	UUID::UUID(uint64_t uuid) noexcept : m_UUID(uuid)
	{}
}
