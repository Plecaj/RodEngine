#include "rdpch.h"
#include "ScriptRuntime.h"

#ifdef RD_PLATFORM_WINDOWS
	#include <Windows.h>
#endif

namespace Rod {

	using hostfxr_handle = void*;
	using hostfxr_initialize_for_runtime_config_fn = int32_t(__cdecl*)(const wchar_t*, const void*, hostfxr_handle*);
	using hostfxr_get_runtime_delegate_fn = int32_t(__cdecl*)(hostfxr_handle, int32_t, void**);
	using hostfxr_close_fn = int32_t(__cdecl*)(hostfxr_handle);
	using load_assembly_and_get_function_pointer_fn = int32_t(__cdecl*)(const wchar_t*, const wchar_t*, const wchar_t*, const wchar_t*, void*, void**);

	static constexpr int32_t s_LoadAssemblyAndGetFunctionPointerDelegate = 5;

	static std::string ToUTF8(const std::wstring& value)
	{
#ifdef RD_PLATFORM_WINDOWS
		if (value.empty())
			return {};

		int size = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, nullptr, 0, nullptr, nullptr);
		if (size <= 1)
			return {};

		std::string result(size - 1, '\0');
		WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, result.data(), size, nullptr, nullptr);
		return result;
#else
		return std::string(value.begin(), value.end());
#endif
	}

	void* ScriptRuntime::LoadHostFxr()
	{
#ifdef RD_PLATFORM_WINDOWS
		std::filesystem::path fxrRoot = "C:/Program Files/dotnet/host/fxr";
		if (!std::filesystem::exists(fxrRoot))
			return nullptr;

		std::filesystem::path newestVersion;
		for (const auto& entry : std::filesystem::directory_iterator(fxrRoot))
		{
			if (!entry.is_directory())
				continue;
			if (newestVersion.empty() || entry.path().filename().wstring() > newestVersion.filename().wstring())
				newestVersion = entry.path();
		}

		if (newestVersion.empty())
			return nullptr;

		std::filesystem::path hostfxrPath = newestVersion / "hostfxr.dll";
		return LoadLibraryW(hostfxrPath.wstring().c_str());
#else
		return nullptr;
#endif
	}

	bool ScriptRuntime::Initialize(const std::filesystem::path& runtimeConfigPath, const std::filesystem::path& coreAssemblyPath)
	{
#ifdef RD_PLATFORM_WINDOWS
		if (IsInitialized())
			return true;

		m_HostFxr = LoadHostFxr();
		if (!m_HostFxr)
		{
			RD_CORE_ERROR("Failed to load hostfxr.");
			return false;
		}

		auto initializeForRuntimeConfig = (hostfxr_initialize_for_runtime_config_fn)GetProcAddress((HMODULE)m_HostFxr, "hostfxr_initialize_for_runtime_config");
		auto getRuntimeDelegate = (hostfxr_get_runtime_delegate_fn)GetProcAddress((HMODULE)m_HostFxr, "hostfxr_get_runtime_delegate");
		auto close = (hostfxr_close_fn)GetProcAddress((HMODULE)m_HostFxr, "hostfxr_close");

		if (!initializeForRuntimeConfig || !getRuntimeDelegate || !close)
		{
			RD_CORE_ERROR("hostfxr does not expose the required hosting functions.");
			Shutdown();
			return false;
		}

		hostfxr_handle context = nullptr;
		int32_t result = initializeForRuntimeConfig(runtimeConfigPath.wstring().c_str(), nullptr, &context);
		if (result != 0 || !context)
		{
			RD_CORE_ERROR("Failed to initialize .NET runtime from '{}'.", runtimeConfigPath.string());
			Shutdown();
			return false;
		}

		result = getRuntimeDelegate(context, s_LoadAssemblyAndGetFunctionPointerDelegate, &m_LoadAssemblyAndGetFunctionPointer);
		close(context);

		if (result != 0 || !m_LoadAssemblyAndGetFunctionPointer)
		{
			RD_CORE_ERROR("Failed to get .NET load_assembly_and_get_function_pointer delegate.");
			Shutdown();
			return false;
		}

		m_CoreAssemblyPath = coreAssemblyPath;
		return true;
#else
		return false;
#endif
	}

	void ScriptRuntime::Shutdown()
	{
#ifdef RD_PLATFORM_WINDOWS
		m_LoadAssemblyAndGetFunctionPointer = nullptr;
		m_CoreAssemblyPath.clear();

		if (m_HostFxr)
		{
			FreeLibrary((HMODULE)m_HostFxr);
			m_HostFxr = nullptr;
		}
#endif
	}

	void* ScriptRuntime::GetFunction(const std::wstring& typeName, const std::wstring& methodName)
	{
		if (!IsInitialized())
			return nullptr;

		void* function = nullptr;
		const wchar_t* unmanagedCallersOnlyMethod = (const wchar_t*)-1;
		auto loadAssemblyAndGetFunctionPointer = (load_assembly_and_get_function_pointer_fn)m_LoadAssemblyAndGetFunctionPointer;
		int32_t result = loadAssemblyAndGetFunctionPointer(
			m_CoreAssemblyPath.wstring().c_str(),
			typeName.c_str(),
			methodName.c_str(),
			unmanagedCallersOnlyMethod,
			nullptr,
			&function);

		if (result != 0 || !function)
		{
			RD_CORE_ERROR("Failed to load managed script function '{}'.", ToUTF8(methodName));
			return nullptr;
		}

		return function;
	}
}
