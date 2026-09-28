#pragma once

#include <filesystem>

namespace Rod {

	class ScriptRuntime
	{
	public:
		bool Initialize(const std::filesystem::path& runtimeConfigPath, const std::filesystem::path& coreAssemblyPath);
		void Shutdown();

		void* GetFunction(const std::wstring& typeName, const std::wstring& methodName);

		bool IsInitialized() const { return m_LoadAssemblyAndGetFunctionPointer != nullptr; }
	private:
		void* LoadHostFxr();
	private:
		void* m_HostFxr = nullptr;
		void* m_LoadAssemblyAndGetFunctionPointer = nullptr;
		std::filesystem::path m_CoreAssemblyPath;
	};
}
