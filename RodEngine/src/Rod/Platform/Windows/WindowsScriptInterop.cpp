#include "rdpch.h"

#include "Rod/Scripting/ScriptInterop.h"

#include <Windows.h> // IWYU pragma: keep

#include <objbase.h> // IWYU pragma: keep

namespace Rod::ScriptInterop {

	auto Allocate(std::size_t size) noexcept -> void*
	{
		return CoTaskMemAlloc(size);
	}

	void Free(void* buffer) noexcept
	{
		CoTaskMemFree(buffer);
	}

	auto ToHostString(const std::string& value) -> std::filesystem::path::string_type
	{
		const auto inputSize = static_cast<int>(value.size());
		const int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), inputSize, nullptr, 0);
		if (size <= 0)
			return {};

		std::wstring result(static_cast<std::size_t>(size), L'\0');
		MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), inputSize, result.data(), size);
		return result;
	}

}
