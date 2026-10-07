#pragma once

#include <filesystem>

namespace Rod {

	class SharedLibrary
	{
	public:
		SharedLibrary() = default;
		~SharedLibrary();
		SharedLibrary(const SharedLibrary&) = delete;
		SharedLibrary& operator=(const SharedLibrary&) = delete;

		bool Load(const std::filesystem::path& path);
		bool IsLoaded() const { return m_Handle != nullptr; }
		void Unload();
		void* GetFunction(const char* name) const;
	private:
		void* m_Handle = nullptr;
	};

}
