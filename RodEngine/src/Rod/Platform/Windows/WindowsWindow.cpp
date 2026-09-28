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

	Window* Window::Create(const WindowProps& props) 
	{
		return new WindowsWindow(props);
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

		if (!s_GLFWInitialized)
		{
			// TODO: glfwTerminate on system shutdown
			RD_PROFILE_SCOPE("glfw init");
			int succes = glfwInit();
			RD_CORE_ASSERT(succes, "Could not initialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);

			s_GLFWInitialized = true;
		}

		{
			RD_PROFILE_SCOPE("glfw create window");
			
			if(props._IsEditor)
				glfwWindowHint(GLFW_TITLEBAR, false);

			m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
		}

		m_Context = new OpenGLContext(m_Window);
		m_Context->Init();

		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		// Set GLFW callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

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
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				KeyTypedEvent event(keycode);
				data.EventCallback(event);

			});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

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
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			
			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});
		
		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);				
			data.EventCallback(event);
		});

		if (props.TaskbarIconFilepath.empty())
			return;

		GLFWimage icon;

		int width, height, channels;
		auto data = stbi_load(props.TaskbarIconFilepath.c_str(), &width, &height, &channels, 4);

		RD_CORE_ASSERT(data, "Failed to load taskbar icon image");

		icon.width = width;
		icon.height = height;
		icon.pixels = data;

		glfwSetWindowIcon(m_Window, 1, &icon);
		stbi_image_free(data);
	}

	void WindowsWindow::Shutdown()
	{
		RD_PROFILE_FUNCTION();

		glfwDestroyWindow(m_Window);
	}

	void WindowsWindow::OnUpdate()
	{
		RD_PROFILE_FUNCTION();

		UpdateWindowDrag();
		glfwPollEvents();
		m_Context->SwapBuffers();
	}

	void WindowsWindow::Minimalize() const
	{
		m_WindowDragActive = false;
		glfwIconifyWindow(m_Window);
	}

	void WindowsWindow::Maximalize() const
	{
		m_WindowDragActive = false;
		glfwMaximizeWindow(m_Window);
	}

	void WindowsWindow::Restore() const
	{
		m_WindowDragActive = false;
		glfwRestoreWindow(m_Window);
	}

	void WindowsWindow::BeginWindowDrag() const
	{
		if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS)
		{
			m_WindowDragActive = false;
			return;
		}

		if (glfwGetWindowAttrib(m_Window, GLFW_MAXIMIZED))
			glfwRestoreWindow(m_Window);

		int windowX, windowY;
		double mouseX, mouseY;
		glfwGetWindowPos(m_Window, &windowX, &windowY);
		glfwGetCursorPos(m_Window, &mouseX, &mouseY);

		glm::vec2 mouseScreen = { (float)windowX + (float)mouseX, (float)windowY + (float)mouseY };

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

	void WindowsWindow::UpdateWindowDrag() const
	{
		if (!m_WindowDragActive)
			return;

		if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS || !glfwGetWindowAttrib(m_Window, GLFW_FOCUSED))
			m_WindowDragActive = false;
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
