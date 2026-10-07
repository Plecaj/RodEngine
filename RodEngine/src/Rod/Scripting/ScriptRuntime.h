#pragma once

#include <filesystem>
#include <string>

namespace Rod {

	class ScriptRuntime
	{
	public:
		bool Initialize(const std::filesystem::path& runtimeConfigPath, const std::filesystem::path& coreAssemblyPath);
		void Shutdown();

		void* GetFunction(const std::string& typeName, const std::string& methodName);

		bool IsInitialized() const { return m_LoadAssemblyAndGetFunctionPointer != nullptr; }
	private:
		void* m_LoadAssemblyAndGetFunctionPointer = nullptr;
		std::filesystem::path m_CoreAssemblyPath;
	};

}
