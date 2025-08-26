#include "rdpch.h"
#include "DotNetRuntime.h"

#ifdef _WIN32
    #include <Windows.h>
#else
    RD_CORE_ASSERT(false, "Only working on windows rn");
#endif
    
// Note that this file is test rather then actual codebase
// Expect it to be simple and not fully correct
namespace Rod{

    DotNetRuntime::DotNetRuntime()
    {
        char_t buffer[MAX_PATH];
        size_t buffer_size = sizeof(buffer) / sizeof(char_t);
        int rc = get_hostfxr_path(buffer, &buffer_size, nullptr);
        if (rc != 0)
            RD_CORE_ASSERT(false, "");

        HMODULE lib = LoadLibraryW(buffer);
        m_InitFunctionPtr = (hostfxr_initialize_for_runtime_config_fn)GetProcAddress(lib, "hostfxr_initialize_for_runtime_config");
        m_GetDelegateFunctionPtr = (hostfxr_get_runtime_delegate_fn)GetProcAddress(lib, "hostfxr_get_runtime_delegate");
        m_CloseFunctionPtr = (hostfxr_close_fn)GetProcAddress(lib, "hostfxr_close");

        RD_CORE_ASSERT(m_InitFunctionPtr && m_GetDelegateFunctionPtr && m_GetDelegateFunctionPtr, "Couldnt load some hostfxr function");
    }
}
