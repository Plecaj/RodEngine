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
		SharedLibrary(SharedLibrary&&) = delete;
		SharedLibrary& operator=(SharedLibrary&&) = delete;

		bool Load(const std::filesystem::path& path);
		[[nodiscard]] bool IsLoaded() const noexcept { return m_Handle != nullptr; }
		void Unload();
		[[nodiscard]] void* GetFunction(const char* name) const;

	private:
		void* m_Handle = nullptr;
	};

}
