#include "rdpch.h"
#include "ScriptRuntime.h"

#include "Rod/Utils/SharedLibrary.h"

#ifdef ROD_HAS_DOTNET_HOST
	#include <nethost.h>
	#include <hostfxr.h>
	#include <coreclr_delegates.h>
	#ifdef RD_PLATFORM_WINDOWS
		#include <Windows.h>
	#endif
#endif

namespace Rod {

#ifdef ROD_HAS_DOTNET_HOST
	static SharedLibrary s_HostFxr;

	static std::basic_string<char_t> ToHostString(const std::string& value)
	{
#ifdef RD_PLATFORM_WINDOWS
		int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), (int)value.size(), nullptr, 0);
		std::wstring result(size, L'\0');
		MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), (int)value.size(), result.data(), size);
		return result;
#else
		return value;
#endif
	}
#endif

	bool ScriptRuntime::Initialize(const std::filesystem::path& runtimeConfigPath, const std::filesystem::path& coreAssemblyPath)
	{
#ifdef ROD_HAS_DOTNET_HOST
		if (IsInitialized())
			return true;

		if (!s_HostFxr.IsLoaded())
		{
			size_t size = 0;
			get_hostfxr_path(nullptr, &size, nullptr);
			std::vector<char_t> hostfxrPath(size);
			if (size == 0 || get_hostfxr_path(hostfxrPath.data(), &size, nullptr) != 0 || !s_HostFxr.Load(hostfxrPath.data()))
			{
				RD_CORE_ERROR("Failed to locate or load hostfxr. Install the .NET 9 runtime.");
				return false;
			}
		}

		auto initializeForRuntimeConfig = (hostfxr_initialize_for_runtime_config_fn)s_HostFxr.GetFunction("hostfxr_initialize_for_runtime_config");
		auto getRuntimeDelegate = (hostfxr_get_runtime_delegate_fn)s_HostFxr.GetFunction("hostfxr_get_runtime_delegate");
		auto close = (hostfxr_close_fn)s_HostFxr.GetFunction("hostfxr_close");

		if (!initializeForRuntimeConfig || !getRuntimeDelegate || !close)
		{
			RD_CORE_ERROR("hostfxr does not expose the required hosting functions.");
			Shutdown();
			return false;
		}

		hostfxr_handle context = nullptr;
		int32_t result = initializeForRuntimeConfig(std::filesystem::absolute(runtimeConfigPath).c_str(), nullptr, &context);
		if (result < 0 || !context)
		{
			if (context)
				close(context);
			RD_CORE_ERROR("Failed to initialize .NET runtime from '{}'.", runtimeConfigPath.string());
			Shutdown();
			return false;
		}

		result = getRuntimeDelegate(context, hdt_load_assembly_and_get_function_pointer, &m_LoadAssemblyAndGetFunctionPointer);
		close(context);

		if (result != 0 || !m_LoadAssemblyAndGetFunctionPointer)
		{
			RD_CORE_ERROR("Failed to get .NET load_assembly_and_get_function_pointer delegate.");
			Shutdown();
			return false;
		}

		m_CoreAssemblyPath = std::filesystem::absolute(coreAssemblyPath);
		return true;
#else
		(void)runtimeConfigPath;
		(void)coreAssemblyPath;
		RD_CORE_ERROR("C# scripting requires a build configured with the .NET 9 SDK.");
		return false;
#endif
	}

	void ScriptRuntime::Shutdown()
	{
		m_LoadAssemblyAndGetFunctionPointer = nullptr;
		m_CoreAssemblyPath.clear();
	}

	void* ScriptRuntime::GetFunction(const std::string& typeName, const std::string& methodName)
	{
#ifdef ROD_HAS_DOTNET_HOST
		if (!IsInitialized())
			return nullptr;

		void* function = nullptr;
		auto loadAssemblyAndGetFunctionPointer = (load_assembly_and_get_function_pointer_fn)m_LoadAssemblyAndGetFunctionPointer;
		int32_t result = loadAssemblyAndGetFunctionPointer(
			m_CoreAssemblyPath.c_str(),
			ToHostString(typeName).c_str(),
			ToHostString(methodName).c_str(),
			UNMANAGEDCALLERSONLY_METHOD,
			nullptr,
			&function);

		if (result != 0 || !function)
		{
			RD_CORE_ERROR("Failed to load managed script function '{}'.", methodName);
			return nullptr;
		}

		return function;
#else
		(void)typeName;
		(void)methodName;
		return nullptr;
#endif
	}

}
