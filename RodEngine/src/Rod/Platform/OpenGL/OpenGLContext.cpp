#include "rdpch.h"

#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <stdexcept>

namespace Rod {

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle)
	{
		RD_PROFILE_FUNCTION();
		if (!windowHandle)
			throw std::invalid_argument("OpenGL window handle required");

		glfwMakeContextCurrent(m_WindowHandle);
		// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
		const int status = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
		if (!status)
			throw std::runtime_error("OpenGL loader initialization failed");

		RD_CORE_INFO("OpenGL Info:");
		RD_CORE_INFO("	Vendor: {0}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
		RD_CORE_INFO("	Renderer: {0}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
		RD_CORE_INFO("	Version: {0}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
		// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(
		    [](GLenum, GLenum, GLuint, GLenum severity, GLsizei, const GLchar* message, const void*) {
			    if (severity == GL_DEBUG_SEVERITY_HIGH)
				    RD_CORE_ERROR("GL DEBUG HIGH: {0}", message);
			    else if (severity == GL_DEBUG_SEVERITY_MEDIUM)
				    RD_CORE_WARN("GL DEBUG MEDIUM: {0}", message);
		    },
		    nullptr);
	}

	void OpenGLContext::SwapBuffers()
	{
		RD_PROFILE_FUNCTION();

		glfwSwapBuffers(m_WindowHandle);
	}

}
