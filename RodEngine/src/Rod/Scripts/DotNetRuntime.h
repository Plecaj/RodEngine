#pragma once

#include <nethost.h>
#include <hostfxr.h>
#include <coreclr_delegates.h>

// Note that this file is test rather then actual codebase
// Expect it to be simple and not fullys correct
namespace Rod{

	class DotNetRuntime 
	{
	public:
		DotNetRuntime();
	private:
		hostfxr_initialize_for_runtime_config_fn m_InitFunctionPtr;
		hostfxr_get_runtime_delegate_fn m_GetDelegateFunctionPtr;
		hostfxr_close_fn m_CloseFunctionPtr;
	};
}