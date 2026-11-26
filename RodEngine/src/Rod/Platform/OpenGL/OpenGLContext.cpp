#include "rdpch.h"

#include "OpenGLContext.h"

#include "Rod/Utils/FunctionPointerCast.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <stdexcept>

namespace Rod {

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle)
	{
		if (!windowHandle)
			throw std::invalid_argument("OpenGL window handle required");

		glfwMakeContextCurrent(m_WindowHandle);
		const int status =
		    gladLoadGLLoader([](const char* name) { return FunctionPointerCast<void*>(glfwGetProcAddress(name)); });
		if (!status)
			throw std::runtime_error("OpenGL loader initialization failed");

		// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
		Log::Core().info("OpenGL Info:");
		Log::Core().info("	Vendor: {0}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
		Log::Core().info("	Renderer: {0}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
		Log::Core().info("	Version: {0}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
		// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(
		    [](GLenum, GLenum, GLuint, GLenum severity, GLsizei, const GLchar* message, const void*) {
			    if (severity == GL_DEBUG_SEVERITY_HIGH)
				    Log::Core().error("GL DEBUG HIGH: {0}", message);
			    else if (severity == GL_DEBUG_SEVERITY_MEDIUM)
				    Log::Core().warn("GL DEBUG MEDIUM: {0}", message);
		    },
		    nullptr);
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}

}
