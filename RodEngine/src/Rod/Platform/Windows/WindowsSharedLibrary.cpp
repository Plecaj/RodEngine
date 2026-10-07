#include "rdpch.h"
#include "Rod/Utils/SharedLibrary.h"

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
			FreeLibrary((HMODULE)m_Handle);
			m_Handle = nullptr;
		}
	}

	void* SharedLibrary::GetFunction(const char* name) const
	{
		return m_Handle ? (void*)GetProcAddress((HMODULE)m_Handle, name) : nullptr;
	}

}
