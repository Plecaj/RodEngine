#include "rdpch.h"
#include "WindowsWindow.h"

#include "Rod/Events/Event.h"
#include "Rod/Events/KeyEvent.h"
#include "Rod/Events/ApplicationEvent.h"
#include "Rod/Events/MouseEvent.h"

#include "Rod/Platform/OpenGL/OpenGLContext.h"

#include <stb_image.h>

namespace Rod {

	static bool s_GLFWInitialized = false;

	static void GLFWErrorCallback(int error, const char* description)
	{
		RD_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	Scope<Window> Window::Create(const WindowProps& props)
	{
		return CreateScope<WindowsWindow>(props);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
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
		SetVSync(true);
		SetGLFWCallbacks();
		SetTaskbarIcon(props.TaskbarIconFilepath);
	}

	void WindowsWindow::InitGLFW()
	{
		if (s_GLFWInitialized)
			return;

		RD_PROFILE_SCOPE("glfw init");
		int success = glfwInit();
		RD_CORE_ASSERT(success, "Could not initialize GLFW!");
		glfwSetErrorCallback(GLFWErrorCallback);

		s_GLFWInitialized = true;
	}

	void WindowsWindow::CreateNativeWindow(const WindowProps& props)
	{
		RD_PROFILE_SCOPE("glfw create window");

		if (props.IsEditor)
			glfwWindowHint(GLFW_TITLEBAR, false);

		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
		RD_CORE_ASSERT(m_Window, "Could not create GLFW window!");
	}

	void WindowsWindow::SetGLFWCallbacks()
	{
		SetWindowCallbacks();
		SetKeyboardCallbacks();
		SetMouseCallbacks();
	}

	void WindowsWindow::SetWindowCallbacks()
	{
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = GetWindowData(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowData& data = GetWindowData(window);

			WindowCloseEvent event;
			data.EventCallback(event);
		});
	}

	void WindowsWindow::SetKeyboardCallbacks()
	{
		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowData& data = GetWindowData(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, 1);
					data.EventCallback(event);
					break;
				} 
			}

		});


		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
		{
			WindowData& data = GetWindowData(window);
			KeyTypedEvent event(keycode);
			data.EventCallback(event);

		});
	}

	void WindowsWindow::SetMouseCallbacks()
	{
		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowData& data = GetWindowData(window);

			switch (action)
			{
			case GLFW_PRESS:
			{
				MouseButtonPressedEvent event(button);
				data.EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleasedEvent event(button);
				data.EventCallback(event);
				break;
			}
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			WindowData& data = GetWindowData(window);
			
			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowData& data = GetWindowData(window);

			MouseMovedEvent event((float)xPos, (float)yPos);				
			data.EventCallback(event);
		});
	}

	void WindowsWindow::SetTaskbarIcon(const std::string& iconFilepath) const
	{
		if (iconFilepath.empty())
			return;

		GLFWimage icon;

		int width, height, channels;
		auto data = stbi_load(iconFilepath.c_str(), &width, &height, &channels, 4);

		RD_CORE_ASSERT(data, "Failed to load taskbar icon image");

		icon.width = width;
		icon.height = height;
		icon.pixels = data;

		glfwSetWindowIcon(m_Window, 1, &icon);
		stbi_image_free(data);
	}

	WindowsWindow::WindowData& WindowsWindow::GetWindowData(GLFWwindow* window)
	{
		return *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
	}

	void WindowsWindow::Shutdown()
	{
		RD_PROFILE_FUNCTION();

		if (m_Window)
			glfwDestroyWindow(m_Window);
	}

	void WindowsWindow::OnUpdate()
	{
		RD_PROFILE_FUNCTION();

		UpdateWindowDrag();
		glfwPollEvents();
		m_Context->SwapBuffers();
	}

	void WindowsWindow::Minimize() const
	{
		ResetWindowDrag();
		glfwIconifyWindow(m_Window);
	}

	void WindowsWindow::Maximize() const
	{
		ResetWindowDrag();
		glfwMaximizeWindow(m_Window);
	}

	void WindowsWindow::Restore() const
	{
		ResetWindowDrag();
		glfwRestoreWindow(m_Window);
	}

	void WindowsWindow::BeginWindowDrag() const
	{
		if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS)
		{
			ResetWindowDrag();
			return;
		}

		if (glfwGetWindowAttrib(m_Window, GLFW_MAXIMIZED))
			glfwRestoreWindow(m_Window);

		int windowX, windowY;
		glfwGetWindowPos(m_Window, &windowX, &windowY);

		glm::vec2 mouseScreen = GetMouseScreenPosition();

		if (!m_WindowDragActive)
		{
			m_WindowDragStartMouseScreen = mouseScreen;
			m_WindowDragStartPosition = { windowX, windowY };
			m_WindowDragActive = true;
		}

		glm::vec2 dragDelta = mouseScreen - m_WindowDragStartMouseScreen;
		int newWinX = m_WindowDragStartPosition.x + (int)dragDelta.x;
		int newWinY = m_WindowDragStartPosition.y + (int)dragDelta.y;
		glfwSetWindowPos(m_Window, newWinX, newWinY);
	}

	void WindowsWindow::ResetWindowDrag() const
	{
		m_WindowDragActive = false;
	}

	void WindowsWindow::UpdateWindowDrag() const
	{
		if (!m_WindowDragActive)
			return;

		if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS || !glfwGetWindowAttrib(m_Window, GLFW_FOCUSED))
			ResetWindowDrag();
	}	

	glm::vec2 WindowsWindow::GetMouseScreenPosition() const
	{
		int windowX, windowY;
		double mouseX, mouseY;
		glfwGetWindowPos(m_Window, &windowX, &windowY);
		glfwGetCursorPos(m_Window, &mouseX, &mouseY);

		return { (float)windowX + (float)mouseX, (float)windowY + (float)mouseY };
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		RD_PROFILE_FUNCTION();

		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}

}
