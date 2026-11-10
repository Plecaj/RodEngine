#pragma once

#include <filesystem>
#include <string>

namespace Rod {

	class SharedLibrary
	{
	public:
		explicit SharedLibrary(const std::filesystem::path& path);
		~SharedLibrary();
		SharedLibrary(const SharedLibrary&) = delete;
		SharedLibrary& operator=(const SharedLibrary&) = delete;
		SharedLibrary(SharedLibrary&& other) noexcept;
		SharedLibrary& operator=(SharedLibrary&& other) noexcept;

		[[nodiscard]] bool IsLoaded() const noexcept { return m_Handle != nullptr; }
		[[nodiscard]] void* GetFunction(const char* name) const;

	private:
		void Close() noexcept;

		void* m_Handle;
	};

}
