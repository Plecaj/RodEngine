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
		auto operator=(const SharedLibrary&) -> SharedLibrary& = delete;
		SharedLibrary(SharedLibrary&& other) noexcept;
		auto operator=(SharedLibrary&& other) noexcept -> SharedLibrary&;

		[[nodiscard]] auto IsLoaded() const noexcept -> bool { return m_Handle != nullptr; }
		[[nodiscard]] auto GetFunction(const char* name) const noexcept -> void*;

	private:
		void Close() noexcept;

		void* m_Handle;
	};

}
