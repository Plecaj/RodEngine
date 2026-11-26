#pragma once

#include <cstddef>
#include <filesystem>
#include <string>

namespace Rod::ScriptInterop {

	[[nodiscard]] auto Allocate(std::size_t size) noexcept -> void*;
	void Free(void* buffer) noexcept;

	[[nodiscard]] auto ToHostString(const std::string& value) -> std::filesystem::path::string_type;

}
