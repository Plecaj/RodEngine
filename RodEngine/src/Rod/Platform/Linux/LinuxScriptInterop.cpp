#include "rdpch.h"

#include "Rod/Scripting/ScriptInterop.h"

#include <cstdlib>

namespace Rod::ScriptInterop {

	auto Allocate(std::size_t size) noexcept -> void*
	{
		// NOLINTNEXTLINE(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory)
		return std::malloc(size);
	}

	void Free(void* buffer) noexcept
	{
		// NOLINTNEXTLINE(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory)
		std::free(buffer);
	}

	auto ToHostString(const std::string& value) -> std::filesystem::path::string_type
	{
		return value;
	}

}
