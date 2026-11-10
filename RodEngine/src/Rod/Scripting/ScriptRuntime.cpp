#include "rdpch.h"

#include "ScriptRuntime.h"

#include <bit>
#include <memory>
#include <stdexcept>

#ifdef ROD_HAS_DOTNET_HOST
	#include <coreclr_delegates.h>
	#include <hostfxr.h>
	#include <nethost.h>
	#ifdef RD_PLATFORM_WINDOWS
		#include <Windows.h> // IWYU pragma: keep
	#endif
#endif

namespace Rod {

	ScriptRuntime::ScriptRuntime(ScriptRuntime&& other) noexcept
	    : m_HostFxr(std::move(other.m_HostFxr)),
	      m_LoadAssemblyAndGetFunctionPointer(std::exchange(other.m_LoadAssemblyAndGetFunctionPointer, nullptr)),
	      m_CoreAssemblyPath(std::move(other.m_CoreAssemblyPath))
	{}

	ScriptRuntime& ScriptRuntime::operator=(ScriptRuntime&& other) noexcept
	{
		if (this != &other)
		{
			m_HostFxr = std::move(other.m_HostFxr);
			m_LoadAssemblyAndGetFunctionPointer = std::exchange(other.m_LoadAssemblyAndGetFunctionPointer, nullptr);
			m_CoreAssemblyPath = std::move(other.m_CoreAssemblyPath);
		}
		return *this;
	}

#ifdef ROD_HAS_DOTNET_HOST
	ScriptRuntime::HostString ScriptRuntime::ToHostString(const std::string& value)
	{
	#ifdef RD_PLATFORM_WINDOWS
		const int size = MultiByteToWideChar(
		    CP_UTF8,
		    MB_ERR_INVALID_CHARS,
		    value.data(),
		    static_cast<int>(value.size()),
		    nullptr,
		    0);
		std::wstring result(static_cast<size_t>(size), L'\0');
		MultiByteToWideChar(
		    CP_UTF8,
		    MB_ERR_INVALID_CHARS,
		    value.data(),
		    static_cast<int>(value.size()),
		    result.data(),
		    size);
		return result;
	#else
		return value;
	#endif
	}
#endif

	std::filesystem::path ScriptRuntime::GetHostFxrPath()
	{
#ifdef ROD_HAS_DOTNET_HOST
		size_t size = 0;
		get_hostfxr_path(nullptr, &size, nullptr);
		std::vector<char_t> path(size);
		if (size == 0 || get_hostfxr_path(path.data(), &size, nullptr) != 0)
			throw std::runtime_error("hostfxr missing; install .NET 9");
		return path.data();
#else
		throw std::runtime_error("Scripting unavailable: .NET host missing");
#endif
	}

	std::shared_ptr<const SharedLibrary> ScriptRuntime::GetHostFxrLibrary()
	{
		static const auto library = std::make_shared<const SharedLibrary>(GetHostFxrPath());
		return library;
	}

	ScriptRuntime::ScriptRuntime(
	    const std::filesystem::path& runtimeConfigPath,
	    const std::filesystem::path& coreAssemblyPath)
	    : m_HostFxr(GetHostFxrLibrary()), m_LoadAssemblyAndGetFunctionPointer(nullptr)
	{
#ifdef ROD_HAS_DOTNET_HOST
		if (runtimeConfigPath.empty() || coreAssemblyPath.empty())
			throw std::runtime_error("Runtime paths required");

		const auto configPath = std::filesystem::absolute(runtimeConfigPath);
		m_CoreAssemblyPath = std::filesystem::absolute(coreAssemblyPath);
		if (!std::filesystem::is_regular_file(configPath))
			throw std::runtime_error("Runtime config unavailable '" + configPath.string() + "'");
		if (!std::filesystem::is_regular_file(m_CoreAssemblyPath))
			throw std::runtime_error("Core assembly unavailable '" + m_CoreAssemblyPath.string() + "'");

		auto initializeForRuntimeConfig = std::bit_cast<hostfxr_initialize_for_runtime_config_fn>(
		    m_HostFxr->GetFunction("hostfxr_initialize_for_runtime_config"));
		auto getRuntimeDelegate =
		    std::bit_cast<hostfxr_get_runtime_delegate_fn>(m_HostFxr->GetFunction("hostfxr_get_runtime_delegate"));
		auto close = std::bit_cast<hostfxr_close_fn>(m_HostFxr->GetFunction("hostfxr_close"));

		if (!initializeForRuntimeConfig || !getRuntimeDelegate || !close)
			throw std::runtime_error("hostfxr symbols missing");

		hostfxr_handle context = nullptr;
		int32_t result =
		    initializeForRuntimeConfig(configPath.c_str(), nullptr, &context);
		std::unique_ptr<void, hostfxr_close_fn> ownedContext(context, close);
		if (result < 0 || !ownedContext)
			throw std::runtime_error("Runtime init failed '" + configPath.string() + "': " + std::to_string(result));

		void* delegate = nullptr;
		result = getRuntimeDelegate(
		    ownedContext.get(),
		    hdt_load_assembly_and_get_function_pointer,
		    &delegate);

		if (result != 0 || !delegate)
			throw std::runtime_error("Runtime delegate unavailable: " + std::to_string(result));

		m_LoadAssemblyAndGetFunctionPointer = delegate;
#else
		(void)runtimeConfigPath;
		(void)coreAssemblyPath;
#endif
	}

	void* ScriptRuntime::GetFunction(const std::string& typeName, const std::string& methodName) const
	{
		if (!m_LoadAssemblyAndGetFunctionPointer)
			return nullptr;

#ifdef ROD_HAS_DOTNET_HOST
		void* function = nullptr;
		auto loadAssemblyAndGetFunctionPointer =
		    std::bit_cast<load_assembly_and_get_function_pointer_fn>(m_LoadAssemblyAndGetFunctionPointer);
		const int32_t result = loadAssemblyAndGetFunctionPointer(
		    m_CoreAssemblyPath.c_str(),
		    ToHostString(typeName).c_str(),
		    ToHostString(methodName).c_str(),
		    UNMANAGEDCALLERSONLY_METHOD,
		    nullptr,
		    &function);

		if (result != 0 || !function)
			return nullptr;

		return function;
#else
		(void)typeName;
		(void)methodName;
		return nullptr;
#endif
	}

}
