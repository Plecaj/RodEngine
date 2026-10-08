#include "rdpch.h"
#include "OpenGLContext.h"

namespace Rod {

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		:m_windowHandle(windowHandle)
	{
		RD_CORE_ASSERT(windowHandle, "Window handle is null!")
	}

	void OpenGLContext::Init()
	{
		RD_PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_windowHandle);
		const int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		RD_CORE_ASSERT(status, "Failed to initialize Glad!");
		if (!status)
		{
			RD_CORE_ERROR("Failed to initialize Glad!");
			return;
		}

		RD_CORE_INFO("OpenGL Info:");
		RD_CORE_INFO("	Vendor: {0}", (const char*)glGetString(GL_VENDOR));
		RD_CORE_INFO("	Renderer: {0}", (const char*)glGetString(GL_RENDERER));
		RD_CORE_INFO("	Version: {0}", (const char*)glGetString(GL_VERSION));

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback([](GLenum, GLenum, GLuint, GLenum severity, GLsizei, const GLchar* message, const void*) {
			if (severity == GL_DEBUG_SEVERITY_HIGH)
				RD_CORE_ERROR("GL DEBUG HIGH: {0}", message);
			else if (severity == GL_DEBUG_SEVERITY_MEDIUM)
				RD_CORE_WARN("GL DEBUG MEDIUM: {0}", message);
			}, nullptr);
	}

	void OpenGLContext::SwapBuffers()
	{
		RD_PROFILE_FUNCTION();

		glfwSwapBuffers(m_windowHandle);
	}

}
