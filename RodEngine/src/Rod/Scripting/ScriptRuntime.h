#pragma once

#include <filesystem>
#include <string>

namespace Rod {
	class SharedLibrary;

	class ScriptRuntime
	{
	public:
		bool Initialize(const std::filesystem::path& runtimeConfigPath, const std::filesystem::path& coreAssemblyPath);
		void Shutdown();

		[[nodiscard]] void* GetFunction(const std::string& typeName, const std::string& methodName);

		[[nodiscard]] bool IsInitialized() const noexcept { return m_LoadAssemblyAndGetFunctionPointer != nullptr; }

	private:
#ifdef RD_PLATFORM_WINDOWS
		using HostString = std::wstring;
#else
		using HostString = std::string;
#endif
		static HostString ToHostString(const std::string& value);
		static SharedLibrary s_HostFxr;

		void* m_LoadAssemblyAndGetFunctionPointer = nullptr;
		std::filesystem::path m_CoreAssemblyPath;
	};

}
