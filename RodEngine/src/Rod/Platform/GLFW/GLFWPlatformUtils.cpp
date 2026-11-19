#include "rdpch.h"

#include "Rod/Utils/PlatformUtils.h"

#include <GLFW/glfw3.h>

namespace Rod {

	auto Platform::GetTime() -> float
	{
		return static_cast<float>(glfwGetTime());
	}

}
