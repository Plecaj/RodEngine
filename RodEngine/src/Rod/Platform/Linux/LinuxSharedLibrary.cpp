#include "rdpch.h"

#include "Rod/Utils/SharedLibrary.h"

#include <dlfcn.h>

#include <stdexcept>

namespace Rod {

	SharedLibrary::SharedLibrary(const std::filesystem::path& path)
	    : m_Handle(dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL))
	{
		if (!m_Handle)
			throw std::runtime_error("Library load failed '" + path.string() + "': " + dlerror());
	}

	void SharedLibrary::Close() noexcept
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
