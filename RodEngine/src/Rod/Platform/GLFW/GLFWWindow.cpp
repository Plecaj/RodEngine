#include "rdpch.h"

#include "GLFWWindow.h"

#include "Rod/Core/Window.h"
#include "Rod/Events/ApplicationEvent.h"
#include "Rod/Events/KeyEvent.h"
#include "Rod/Events/MouseEvent.h"
#include "Rod/Platform/OpenGL/OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glm/ext/vector_float2.hpp>

#include <stb_image.h>
#include <stdexcept>

namespace Rod {

	static uint32_t g_WindowCount = 0;

	static void GLFWErrorCallback(int error, const char* description)
	{
		RD_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	Scope<Window> Window::Create(const WindowProps& props)
	{
		return CreateScope<GLFWWindow>(props);
	}

	GLFWWindow::GLFWWindow(const WindowProps& props)
	{
		Init(props);
	}

	GLFWWindow::~GLFWWindow()
	{
		Shutdown();
	}

	void GLFWWindow::Init(const WindowProps& props)
	{
		RD_PROFILE_FUNCTION();

		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		RD_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		InitGLFW();
		CreateNativeWindow(props);

		m_Context = CreateScope<OpenGLContext>(m_Window);
		m_Context->Init();

		glfwSetWindowUserPointer(m_Window, &m_Data);
		GLFWWindow::SetVSync(true);
		SetGLFWCallbacks();
		SetTaskbarIcon(props.TaskbarIconFilepath);
	}

	void GLFWWindow::InitGLFW()
	{
		if (g_WindowCount > 0)
			return;

		RD_PROFILE_SCOPE("glfw init");
		glfwSetErrorCallback(GLFWErrorCallback);
		if (glfwInit() == GLFW_FALSE)
			throw std::runtime_error("Could not initialize GLFW.");
	}

	void GLFWWindow::CreateNativeWindow(const WindowProps& props)
	{
		RD_PROFILE_SCOPE("glfw create window");

		glfwDefaultWindowHints();
		constexpr int openGLMajorVersion = 4;
		constexpr int openGLMinorVersion = 6;
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, openGLMajorVersion);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, openGLMinorVersion);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		if (props.IsEditor)
#ifdef RD_PLATFORM_WINDOWS
			glfwWindowHint(GLFW_TITLEBAR, GLFW_FALSE);
#else
			glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
#endif

		m_Window = glfwCreateWindow(
		    static_cast<int>(props.Width),
		    static_cast<int>(props.Height),
		    m_Data.Title.c_str(),
		    nullptr,
		    nullptr);
		if (!m_Window)
		{
			if (g_WindowCount == 0)
				glfwTerminate();
			throw std::runtime_error("Could not create an OpenGL 4.6 GLFW window.");
		}
		++g_WindowCount;
	}

	void GLFWWindow::SetGLFWCallbacks()
	{
		SetWindowCallbacks();
		SetKeyboardCallbacks();
		SetMouseCallbacks();
	}

	void GLFWWindow::SetWindowCallbacks()
	{
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
			WindowData& data = GetWindowData(window);
			data.Width = static_cast<unsigned int>(width);
			data.Height = static_cast<unsigned int>(height);

			WindowResizeEvent event(data.Width, data.Height);
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
			const WindowData& data = GetWindowData(window);

			WindowCloseEvent event;
			data.EventCallback(event);
		});
	}

	void GLFWWindow::SetKeyboardCallbacks()
	{
		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int, int action, int) {
			const WindowData& data = GetWindowData(window);

			switch (action)
			{
				case GLFW_PRESS: {
					KeyPressedEvent event(key, false);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE: {
					KeyReleasedEvent event(key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT: {
					KeyPressedEvent event(key, true);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode) {
			const WindowData& data = GetWindowData(window);
			KeyTypedEvent event(static_cast<int>(keycode));
			data.EventCallback(event);
		});
	}

	void GLFWWindow::SetMouseCallbacks()
	{
		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int) {
			const WindowData& data = GetWindowData(window);

			switch (action)
			{
				case GLFW_PRESS: {
					MouseButtonPressedEvent event(button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE: {
					MouseButtonReleasedEvent event(button);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
			const WindowData& data = GetWindowData(window);

			MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) {
			const WindowData& data = GetWindowData(window);

			MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
			data.EventCallback(event);
		});
	}

	void GLFWWindow::SetTaskbarIcon(const std::string& iconFilepath) const
	{
		if (iconFilepath.empty())
			return;

		GLFWimage icon;

		int width{}, height{}, channels{};
		auto data = stbi_load(iconFilepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

		if (!data)
		{
			RD_CORE_WARN("Failed to load taskbar icon '{}'.", iconFilepath);
			return;
		}

		icon.width = width;
		icon.height = height;
		icon.pixels = data;

		glfwSetWindowIcon(m_Window, 1, &icon);
		stbi_image_free(data);
	}

	GLFWWindow::WindowData& GLFWWindow::GetWindowData(GLFWwindow* window)
	{
		return *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
	}

	void GLFWWindow::Shutdown()
	{
		RD_PROFILE_FUNCTION();

		m_Context.reset();
		if (m_Window)
		{
			glfwDestroyWindow(m_Window);
			m_Window = nullptr;
			if (--g_WindowCount == 0)
				glfwTerminate();
		}
	}

	void GLFWWindow::OnUpdate()
	{
		RD_PROFILE_FUNCTION();

		UpdateWindowDrag();
		glfwPollEvents();
		m_Context->SwapBuffers();
	}

	void GLFWWindow::Minimize() const
	{
		ResetWindowDrag();
		glfwIconifyWindow(m_Window);
	}

	void GLFWWindow::Maximize() const
	{
		ResetWindowDrag();
		glfwMaximizeWindow(m_Window);
	}

	void GLFWWindow::Restore() const
	{
		ResetWindowDrag();
		glfwRestoreWindow(m_Window);
	}

	void GLFWWindow::BeginWindowDrag() const
	{
		if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS)
		{
			ResetWindowDrag();
			return;
		}

		if (glfwGetWindowAttrib(m_Window, GLFW_MAXIMIZED) != GLFW_FALSE)
			glfwRestoreWindow(m_Window);

		int windowX{}, windowY{};
		glfwGetWindowPos(m_Window, &windowX, &windowY);

		const glm::vec2 mouseScreen = GetMouseScreenPosition();

		if (!m_WindowDragActive)
		{
			m_WindowDragStartMouseScreen = mouseScreen;
			m_WindowDragStartPosition = {windowX, windowY};
			m_WindowDragActive = true;
		}

		const glm::vec2 dragDelta = mouseScreen - m_WindowDragStartMouseScreen;
		const int newWinX = m_WindowDragStartPosition.x + static_cast<int>(dragDelta.x);
		const int newWinY = m_WindowDragStartPosition.y + static_cast<int>(dragDelta.y);
		glfwSetWindowPos(m_Window, newWinX, newWinY);
	}

	void GLFWWindow::ResetWindowDrag() const
	{
		m_WindowDragActive = false;
	}

	void GLFWWindow::UpdateWindowDrag() const
	{
		if (!m_WindowDragActive)
			return;

		if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS ||
		    glfwGetWindowAttrib(m_Window, GLFW_FOCUSED) == GLFW_FALSE)
			ResetWindowDrag();
	}

	glm::vec2 GLFWWindow::GetMouseScreenPosition() const
	{
		int windowX{}, windowY{};
		double mouseX{}, mouseY{};
		glfwGetWindowPos(m_Window, &windowX, &windowY);
		glfwGetCursorPos(m_Window, &mouseX, &mouseY);

		return {
		    static_cast<float>(windowX) + static_cast<float>(mouseX),
		    static_cast<float>(windowY) + static_cast<float>(mouseY)};
	}

	void GLFWWindow::SetVSync(bool enabled)
	{
		RD_PROFILE_FUNCTION();

		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.VSync = enabled;
	}

	bool GLFWWindow::IsVSync() const noexcept
	{
		return m_Data.VSync;
	}

}
