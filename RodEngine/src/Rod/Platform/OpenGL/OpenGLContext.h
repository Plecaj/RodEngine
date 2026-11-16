#pragma once

#include "Rod/Renderer/GraphicsContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Rod {

	class OpenGLContext : public GraphicsContext
	{
	public:
		explicit OpenGLContext(GLFWwindow* windowHandle);
		~OpenGLContext() override = default;
		OpenGLContext(const OpenGLContext&) = delete;
		OpenGLContext& operator=(const OpenGLContext&) = delete;
		OpenGLContext(OpenGLContext&&) = delete;
		OpenGLContext& operator=(OpenGLContext&&) = delete;

		virtual void SwapBuffers() override;

	private:
		GLFWwindow* m_WindowHandle;
	};

}
