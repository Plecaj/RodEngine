#include "rdpch.h"

#include "GLFWWindow.h"

#include "Rod/Core/Window.h"
#include "Rod/Platform/OpenGL/OpenGLContext.h"
#include "Rod/Platform/StbImage.h"

#include <GLFW/glfw3.h>
#include <glm/ext/vector_float2.hpp>

#include <stdexcept>

namespace Rod {

	namespace {
		void GLFWErrorCallback(int error, const char* description)
		{
			Log::Core().error("GLFW Error ({0}): {1}", error, description);
		}

		[[nodiscard]] auto GetMods(int mods) noexcept -> Modifiers
		{
			const auto mask = static_cast<unsigned int>(mods);
			return {
			    .Control = (mask & static_cast<unsigned int>(GLFW_MOD_CONTROL)) != 0U,
			    .Shift = (mask & static_cast<unsigned int>(GLFW_MOD_SHIFT)) != 0U,
			    .Alt = (mask & static_cast<unsigned int>(GLFW_MOD_ALT)) != 0U};
		}

	}

	void GLFWWindow::InitGLFW()
	{
		if (s_WindowCount > 0)
			return;

		glfwSetErrorCallback(GLFWErrorCallback);
		if (glfwInit() == GLFW_FALSE)
			throw std::runtime_error("Could not initialize GLFW.");
	}

	auto Window::Create(const WindowProps& props) -> Scope<Window>
	{
		return CreateScope<GLFWWindow>(props);
	}

	GLFWWindow::GLFWWindow(const WindowProps& props)
	{
		Init(props);
	}

	void GLFWWindow::NativeWindowDeleter::operator()(GLFWwindow* window) const noexcept
	{
		glfwDestroyWindow(window);
		if (--s_WindowCount == 0)
			glfwTerminate();
	}

	void GLFWWindow::Init(const WindowProps& props)
	{
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		Log::Core().info("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		InitGLFW();
		CreateNativeWindow(props);

		m_Context = CreateScope<OpenGLContext>(m_Window.get());

		glfwSetWindowUserPointer(m_Window.get(), this);
		double x{}, y{};
		glfwGetCursorPos(m_Window.get(), &x, &y);
		m_Cursor = {static_cast<float>(x), static_cast<float>(y)};
		GLFWWindow::SetVSync(true);
		SetGLFWCallbacks();
		SetTaskbarIcon(props.TaskbarIconFilepath);
	}

	void GLFWWindow::CreateNativeWindow(const WindowProps& props)
	{
		glfwDefaultWindowHints();
		constexpr int openGLMajorVersion = 4;
		constexpr int openGLMinorVersion = 6;
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, openGLMajorVersion);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, openGLMinorVersion);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, props.Resizable ? GLFW_TRUE : GLFW_FALSE);
		if (!props.NativeTitlebar)
#ifdef RD_PLATFORM_WINDOWS
			glfwWindowHint(GLFW_TITLEBAR, GLFW_FALSE);
#else
			glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
#endif

		m_Window.reset(glfwCreateWindow(
		    static_cast<int>(props.Width),
		    static_cast<int>(props.Height),
		    m_Data.Title.c_str(),
		    nullptr,
		    nullptr));
		if (!m_Window)
		{
			if (s_WindowCount == 0)
				glfwTerminate();
			throw std::runtime_error("Could not create an OpenGL 4.6 GLFW window.");
		}
		++s_WindowCount;

		if (props.Centered)
		{
			constexpr int centerDivisor = 2;
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			int monitorX{}, monitorY{};
			glfwGetMonitorPos(monitor, &monitorX, &monitorY);
			glfwSetWindowPos(
			    m_Window.get(),
			    monitorX + (mode->width - static_cast<int>(props.Width)) / centerDivisor,
			    monitorY + (mode->height - static_cast<int>(props.Height)) / centerDivisor);
		}
	}

	void GLFWWindow::SetGLFWCallbacks()
	{
		SetWindowCallbacks();
		SetKeyboardCallbacks();
		SetMouseCallbacks();
	}

	void GLFWWindow::SetWindowCallbacks()
	{
		glfwSetWindowSizeCallback(m_Window.get(), [](GLFWwindow* window, int width, int height) {
			auto& data = From(window).m_Data;
			data.Width = static_cast<unsigned int>(width);
			data.Height = static_cast<unsigned int>(height);
		});
		glfwSetFramebufferSizeCallback(m_Window.get(), [](GLFWwindow* window, int width, int height) {
			From(window).Queue(
			    FramebufferResized{{static_cast<unsigned int>(width), static_cast<unsigned int>(height)}});
		});
		glfwSetWindowCloseCallback(m_Window.get(), [](GLFWwindow* window) { From(window).Queue(CloseRequested{}); });
		glfwSetWindowFocusCallback(m_Window.get(), [](GLFWwindow* window, int focused) {
			From(window).Queue(FocusChanged{focused == GLFW_TRUE});
		});
	}

	void GLFWWindow::SetKeyboardCallbacks()
	{
		// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
		glfwSetKeyCallback(m_Window.get(), [](GLFWwindow* window, int key, int, int action, int mods) {
			KeyAction keyAction{};
			switch (action)
			{
				case GLFW_PRESS:
					keyAction = KeyAction::Press;
					break;
				case GLFW_RELEASE:
					keyAction = KeyAction::Release;
					break;
				case GLFW_REPEAT:
					keyAction = KeyAction::Repeat;
					break;
				default:
					return;
			}
			From(window).Queue(KeyChanged{.Key = key, .Action = keyAction, .Mods = GetMods(mods)});
		});
	}

	void GLFWWindow::SetMouseCallbacks()
	{
		// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
		glfwSetMouseButtonCallback(m_Window.get(), [](GLFWwindow* window, int button, int action, int mods) {
			auto& self = From(window);
			self.Queue(
			    MouseButtonChanged{
			        .Button = button,
			        .Down = action == GLFW_PRESS,
			        .Mods = GetMods(mods),
			        .Position = self.m_Cursor});
		});
		glfwSetScrollCallback(m_Window.get(), [](GLFWwindow* window, double x, double y) {
			auto& self = From(window);
			self.Queue(Scrolled{.Offset = {static_cast<float>(x), static_cast<float>(y)}, .Position = self.m_Cursor});
		});
		glfwSetCursorPosCallback(m_Window.get(), [](GLFWwindow* window, double x, double y) {
			From(window).m_Cursor = {static_cast<float>(x), static_cast<float>(y)};
		});
	}

	void GLFWWindow::Queue(Event event) noexcept
	{
		try
		{
			m_Pending.push_back(event);
		}
		catch (...)
		{
			m_EventError = std::current_exception();
		}
	}

	void GLFWWindow::SetTaskbarIcon(const std::string& iconFilepath) const
	{
		if (iconFilepath.empty())
			return;

		GLFWimage icon;

		int width{}, height{}, channels{};
		const ImagePixels data(stbi_load(iconFilepath.c_str(), &width, &height, &channels, STBI_rgb_alpha));

		if (!data)
		{
			Log::Core().warn("Failed to load taskbar icon '{}'.", iconFilepath);
			return;
		}

		icon.width = width;
		icon.height = height;
		icon.pixels = data.get();

		glfwSetWindowIcon(m_Window.get(), 1, &icon);
	}

	auto GLFWWindow::From(GLFWwindow* window) noexcept -> GLFWWindow&
	{
		return *static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
	}

	auto GLFWWindow::PollEvents(bool wait) -> std::span<const Event>
	{
		constexpr double idleTimeout = 0.05;
		if (wait && m_Pending.empty() && !m_EventError)
			glfwWaitEventsTimeout(idleTimeout);
		else
			glfwPollEvents();

		if (m_EventError)
			std::rethrow_exception(m_EventError);
		UpdateWindowDrag();
		m_Events.clear();
		m_Events.swap(m_Pending);
		return m_Events;
	}

	void GLFWWindow::Present()
	{
		m_Context->SwapBuffers();
	}

	auto GLFWWindow::GetFramebufferSize() const noexcept -> glm::uvec2
	{
		int width{}, height{};
		glfwGetFramebufferSize(m_Window.get(), &width, &height);
		return {static_cast<unsigned int>(width), static_cast<unsigned int>(height)};
	}

	void GLFWWindow::Minimize() const
	{
		ResetWindowDrag();
		glfwIconifyWindow(m_Window.get());
	}

	void GLFWWindow::Maximize() const
	{
		ResetWindowDrag();
		glfwMaximizeWindow(m_Window.get());
	}

	auto GLFWWindow::IsMinimized() const noexcept -> bool
	{
		return glfwGetWindowAttrib(m_Window.get(), GLFW_ICONIFIED) == GLFW_TRUE;
	}

	auto GLFWWindow::IsMaximized() const noexcept -> bool
	{
		return glfwGetWindowAttrib(m_Window.get(), GLFW_MAXIMIZED) == GLFW_TRUE;
	}

	void GLFWWindow::Restore() const
	{
		ResetWindowDrag();
		glfwRestoreWindow(m_Window.get());
	}

	void GLFWWindow::BeginWindowDrag() const
	{
		if (glfwGetMouseButton(m_Window.get(), GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS)
		{
			ResetWindowDrag();
			return;
		}

		if (IsMaximized())
			Restore();

		int windowX{}, windowY{};
		glfwGetWindowPos(m_Window.get(), &windowX, &windowY);

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
		glfwSetWindowPos(m_Window.get(), newWinX, newWinY);
	}

	void GLFWWindow::ResetWindowDrag() const
	{
		m_WindowDragActive = false;
	}

	void GLFWWindow::UpdateWindowDrag() const
	{
		if (!m_WindowDragActive)
			return;

		if (glfwGetMouseButton(m_Window.get(), GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS ||
		    glfwGetWindowAttrib(m_Window.get(), GLFW_FOCUSED) == GLFW_FALSE)
			ResetWindowDrag();
	}

	auto GLFWWindow::GetMouseScreenPosition() const -> glm::vec2
	{
		int windowX{}, windowY{};
		double mouseX{}, mouseY{};
		glfwGetWindowPos(m_Window.get(), &windowX, &windowY);
		glfwGetCursorPos(m_Window.get(), &mouseX, &mouseY);

		return {
		    static_cast<float>(windowX) + static_cast<float>(mouseX),
		    static_cast<float>(windowY) + static_cast<float>(mouseY)};
	}

	void GLFWWindow::SetVSync(bool enabled)
	{
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.VSync = enabled;
	}

	auto GLFWWindow::IsVSync() const noexcept -> bool
	{
		return m_Data.VSync;
	}

}
