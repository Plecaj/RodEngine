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
		auto operator=(const OpenGLContext&) -> OpenGLContext& = delete;
		OpenGLContext(OpenGLContext&&) = delete;
		auto operator=(OpenGLContext&&) -> OpenGLContext& = delete;

		void SwapBuffers() override;

	private:
		GLFWwindow* m_WindowHandle;
	};

}
