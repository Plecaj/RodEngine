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
		auto operator=(const ScriptRuntime&) -> ScriptRuntime& = delete;
		ScriptRuntime(ScriptRuntime&& other) noexcept;
		auto operator=(ScriptRuntime&& other) noexcept -> ScriptRuntime&;

		[[nodiscard]] auto GetFunction(const std::string& typeName, const std::string& methodName) const -> void*;

	private:
#ifdef RD_PLATFORM_WINDOWS
		using HostString = std::wstring;
#else
		using HostString = std::string;
#endif
		[[nodiscard]] static auto ToHostString(const std::string& value) -> HostString;

		std::shared_ptr<const SharedLibrary> m_HostFxr;
		void* m_LoadAssemblyAndGetFunctionPointer = nullptr;
		std::filesystem::path m_CoreAssemblyPath;
	};

}
