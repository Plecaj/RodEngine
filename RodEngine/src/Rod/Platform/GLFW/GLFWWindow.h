#pragma once

#include "Rod/Core/Window.h"
#include "Rod/Renderer/GraphicsContext.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace Rod {

	class GLFWWindow : public Window
	{
	public:
		explicit GLFWWindow(const WindowProps& props);
		~GLFWWindow() override = default;
		GLFWWindow(const GLFWWindow&) = delete;
		auto operator=(const GLFWWindow&) -> GLFWWindow& = delete;
		GLFWWindow(GLFWWindow&&) = delete;
		auto operator=(GLFWWindow&&) -> GLFWWindow& = delete;

		void OnUpdate() override;

		[[nodiscard]] auto GetWidth() const noexcept -> unsigned int override { return m_Data.Width; }
		[[nodiscard]] auto GetHeight() const noexcept -> unsigned int override { return m_Data.Height; }

		void Minimize() const override;
		void Maximize() const override;
		[[nodiscard]] auto IsMinimized() const noexcept -> bool override;
		[[nodiscard]] auto IsMaximized() const noexcept -> bool override;
		void Restore() const override;
		void BeginWindowDrag() const override;

		void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		[[nodiscard]] auto IsVSync() const noexcept -> bool override;

		[[nodiscard]] auto GetNativeWindow() const noexcept -> void* override { return m_Window.get(); };

	private:
		void Init(const WindowProps& props);
		void CreateNativeWindow(const WindowProps& props);
		void SetGLFWCallbacks();
		void SetWindowCallbacks();
		void SetKeyboardCallbacks();
		void SetMouseCallbacks();
		void SetTaskbarIcon(const std::string& iconFilepath) const;
		void ResetWindowDrag() const;
		void UpdateWindowDrag() const;
		[[nodiscard]] auto GetMouseScreenPosition() const -> glm::vec2;

	private:
		struct WindowData
		{
			std::string Title;
			unsigned int Width = 0, Height = 0;
			bool VSync = false;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
		struct NativeWindowDeleter
		{
			void operator()(GLFWwindow* window) const noexcept;
		};
		std::unique_ptr<GLFWwindow, NativeWindowDeleter> m_Window;
		Scope<GraphicsContext> m_Context;
		mutable bool m_WindowDragActive = false;
		mutable glm::vec2 m_WindowDragStartMouseScreen = {0.0f, 0.0f};
		mutable glm::ivec2 m_WindowDragStartPosition = {0, 0};

		[[nodiscard]] static auto GetWindowData(GLFWwindow* window) -> WindowData&;
	};

}
