#include "rdpch.h"

#include "ScriptRuntime.h"

#include "Rod/Utils/FunctionPointerCast.h"
#include "ScriptInterop.h"

#include <coreclr_delegates.h>
#include <hostfxr.h>
#include <nethost.h>

#include <memory>
#include <stdexcept>

namespace Rod {

	namespace {

		[[nodiscard]] auto GetHostFxrPath() -> std::filesystem::path
		{
			size_t size = 0;
			get_hostfxr_path(nullptr, &size, nullptr);
			std::vector<char_t> path(size);
			if (size == 0 || get_hostfxr_path(path.data(), &size, nullptr) != 0)
				throw std::runtime_error("hostfxr missing; install .NET 9");
			return path.data();
		}

		[[nodiscard]] auto GetHostFxrLibrary() -> std::shared_ptr<const SharedLibrary>
		{
			static const auto library = std::make_shared<const SharedLibrary>(GetHostFxrPath());
			return library;
		}
	}

	ScriptRuntime::ScriptRuntime(ScriptRuntime&& other) noexcept
	    : m_HostFxr(std::move(other.m_HostFxr)),
	      m_LoadAssemblyAndGetFunctionPointer(std::exchange(other.m_LoadAssemblyAndGetFunctionPointer, nullptr)),
	      m_CoreAssemblyPath(std::move(other.m_CoreAssemblyPath))
	{}

	auto ScriptRuntime::operator=(ScriptRuntime&& other) noexcept -> ScriptRuntime&
	{
		if (this != &other)
		{
			m_HostFxr = std::move(other.m_HostFxr);
			m_LoadAssemblyAndGetFunctionPointer = std::exchange(other.m_LoadAssemblyAndGetFunctionPointer, nullptr);
			m_CoreAssemblyPath = std::move(other.m_CoreAssemblyPath);
		}
		return *this;
	}

	ScriptRuntime::ScriptRuntime(
	    const std::filesystem::path& runtimeConfigPath,
	    const std::filesystem::path& coreAssemblyPath)
	    : m_HostFxr(GetHostFxrLibrary())
	{
		if (runtimeConfigPath.empty() || coreAssemblyPath.empty())
			throw std::runtime_error("Runtime paths required");

		const auto configPath = std::filesystem::absolute(runtimeConfigPath);
		m_CoreAssemblyPath = std::filesystem::absolute(coreAssemblyPath);
		if (!std::filesystem::is_regular_file(configPath))
			throw std::runtime_error("Runtime config unavailable '" + configPath.string() + "'");
		if (!std::filesystem::is_regular_file(m_CoreAssemblyPath))
			throw std::runtime_error("Core assembly unavailable '" + m_CoreAssemblyPath.string() + "'");

		const auto initializeForRuntimeConfig = FunctionPointerCast<hostfxr_initialize_for_runtime_config_fn>(
		    m_HostFxr->GetFunction("hostfxr_initialize_for_runtime_config"));
		const auto getRuntimeDelegate = FunctionPointerCast<hostfxr_get_runtime_delegate_fn>(
		    m_HostFxr->GetFunction("hostfxr_get_runtime_delegate"));
		const auto close = FunctionPointerCast<hostfxr_close_fn>(m_HostFxr->GetFunction("hostfxr_close"));

		if (!initializeForRuntimeConfig || !getRuntimeDelegate || !close)
			throw std::runtime_error("hostfxr symbols missing");

		hostfxr_handle context = nullptr;
		int32_t result = initializeForRuntimeConfig(configPath.c_str(), nullptr, &context);
		const std::unique_ptr<void, hostfxr_close_fn> ownedContext(context, close);
		if (result < 0 || !ownedContext)
			throw std::runtime_error("Runtime init failed '" + configPath.string() + "': " + std::to_string(result));

		void* delegate = nullptr;
		result = getRuntimeDelegate(ownedContext.get(), hdt_load_assembly_and_get_function_pointer, &delegate);

		if (result != 0 || !delegate)
			throw std::runtime_error("Runtime delegate unavailable: " + std::to_string(result));

		m_LoadAssemblyAndGetFunctionPointer = delegate;
	}

	auto ScriptRuntime::GetFunction(const std::string& typeName, const std::string& methodName) const -> void*
	{
		if (!m_LoadAssemblyAndGetFunctionPointer)
			return nullptr;

		// hostfxr requires it
		// NOLINTNEXTLINE(misc-const-correctness)
		void* function = nullptr;
		const auto loadAssemblyAndGetFunctionPointer =
		    FunctionPointerCast<load_assembly_and_get_function_pointer_fn>(m_LoadAssemblyAndGetFunctionPointer);
		const int32_t result = loadAssemblyAndGetFunctionPointer(
		    m_CoreAssemblyPath.c_str(),
		    ScriptInterop::ToHostString(typeName).c_str(),
		    ScriptInterop::ToHostString(methodName).c_str(),
		    UNMANAGEDCALLERSONLY_METHOD,
		    nullptr,
		    &function);

		if (result != 0 || !function)
			return nullptr;

		return function;
	}

}
