#include "rdpch.h"

#include "Rod/Utils/FunctionPointerCast.h"
#include "Rod/Utils/SharedLibrary.h"

// NOLINTBEGIN(misc-include-cleaner)
#include <Windows.h>

#include <system_error>

namespace Rod {

	SharedLibrary::SharedLibrary(const std::filesystem::path& path) : m_Handle(LoadLibraryW(path.c_str()))
	{
		if (!m_Handle)
		{
			const auto error = static_cast<int>(GetLastError());
			throw std::system_error(error, std::system_category(), "Library load failed '" + path.string() + "'");
		}
	}

	void SharedLibrary::Close() noexcept
	{
		if (m_Handle)
		{
			FreeLibrary(static_cast<HMODULE>(m_Handle));
			m_Handle = nullptr;
		}
	}

	auto SharedLibrary::GetFunction(const char* name) const noexcept -> void*
	{
		return m_Handle ? FunctionPointerCast<void*>(GetProcAddress(static_cast<HMODULE>(m_Handle), name)) : nullptr;
	}

}
// NOLINTEND(misc-include-cleaner)
