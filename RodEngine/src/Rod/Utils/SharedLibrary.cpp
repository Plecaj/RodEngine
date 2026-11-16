#include "rdpch.h"

#include "SharedLibrary.h"

#include <utility>

namespace Rod {

	SharedLibrary::~SharedLibrary()
	{
		Close();
	}

	SharedLibrary::SharedLibrary(SharedLibrary&& other) noexcept
	    : m_Handle(std::exchange(other.m_Handle, nullptr))
	{}

	SharedLibrary& SharedLibrary::operator=(SharedLibrary&& other) noexcept
	{
		if (this != &other)
		{
			Close();
			m_Handle = std::exchange(other.m_Handle, nullptr);
		}
		return *this;
	}

}
