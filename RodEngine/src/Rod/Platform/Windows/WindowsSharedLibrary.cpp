#include "rdpch.h"

#include "Rod/Utils/SharedLibrary.h"

// NOLINTBEGIN(misc-include-cleaner,cppcoreguidelines-pro-type-reinterpret-cast)
#include <Windows.h>

namespace Rod {

	SharedLibrary::~SharedLibrary()
	{
		Unload();
	}

	bool SharedLibrary::Load(const std::filesystem::path& path)
	{
		Unload();
		m_Handle = LoadLibraryW(path.c_str());
		return m_Handle != nullptr;
	}

	void SharedLibrary::Unload()
	{
		if (m_Handle)
		{
			FreeLibrary(static_cast<HMODULE>(m_Handle));
			m_Handle = nullptr;
		}
	}

	void* SharedLibrary::GetFunction(const char* name) const
	{
		return m_Handle ? reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(m_Handle), name)) : nullptr;
	}

}
// NOLINTEND(misc-include-cleaner,cppcoreguidelines-pro-type-reinterpret-cast)
