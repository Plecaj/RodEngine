#include "rdpch.h"
#include "Rod/Utils/SharedLibrary.h"

#include <dlfcn.h>

namespace Rod {

	SharedLibrary::~SharedLibrary()
	{
		Unload();
	}

	bool SharedLibrary::Load(const std::filesystem::path& path)
	{
		Unload();
		m_Handle = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
		if (!m_Handle)
			RD_CORE_ERROR("Failed to load library '{}': {}", path.string(), dlerror());
		return m_Handle != nullptr;
	}

	void SharedLibrary::Unload()
	{
		if (m_Handle)
		{
			dlclose(m_Handle);
			m_Handle = nullptr;
		}
	}

	void* SharedLibrary::GetFunction(const char* name) const
	{
		return m_Handle ? dlsym(m_Handle, name) : nullptr;
	}

}
