#include "rdpch.h"
#include "Rod/Utils/PlatformUtils.h"

#include <GLFW/glfw3.h>

namespace Rod {

	float Platform::GetTime()
	{
		return (float)glfwGetTime();
	}

}
