#pragma once

#include "Rod/Utils/SharedLibrary.h"

#include <filesystem>
#include <memory>
#include <string>

namespace Rod {
	class ScriptRuntime
	{
	public:
		ScriptRuntime(const std::filesystem::path& runtimeConfigPath, const std::filesystem::path& coreAssemblyPath);
		~ScriptRuntime() = default;
		ScriptRuntime(const ScriptRuntime&) = delete;
		ScriptRuntime& operator=(const ScriptRuntime&) = delete;
		ScriptRuntime(ScriptRuntime&& other) noexcept;
		ScriptRuntime& operator=(ScriptRuntime&& other) noexcept;

		[[nodiscard]] void* GetFunction(const std::string& typeName, const std::string& methodName) const;

	private:
		static std::filesystem::path GetHostFxrPath();
		static std::shared_ptr<const SharedLibrary> GetHostFxrLibrary();

#ifdef RD_PLATFORM_WINDOWS
		using HostString = std::wstring;
#else
		using HostString = std::string;
#endif
		static HostString ToHostString(const std::string& value);

		std::shared_ptr<const SharedLibrary> m_HostFxr;
		void* m_LoadAssemblyAndGetFunctionPointer;
		std::filesystem::path m_CoreAssemblyPath;
	};

}
