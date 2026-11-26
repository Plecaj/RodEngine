#pragma once

#include "Rod/Core/Window.h"
#include "Rod/Renderer/GraphicsContext.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <exception>
#include <vector>

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

		[[nodiscard]] auto PollEvents(bool wait) -> std::span<const Event> override;
		void Present() override;

		[[nodiscard]] auto GetWidth() const noexcept -> unsigned int override { return m_Data.Width; }
		[[nodiscard]] auto GetHeight() const noexcept -> unsigned int override { return m_Data.Height; }

		[[nodiscard]] auto GetFramebufferSize() const noexcept -> glm::uvec2 override;
		[[nodiscard]] auto GetCursor() const noexcept -> glm::vec2 override { return m_Cursor; }
		void Minimize() const override;
		void Maximize() const override;
		[[nodiscard]] auto IsMinimized() const noexcept -> bool override;
		[[nodiscard]] auto IsMaximized() const noexcept -> bool override;
		void Restore() const override;
		void BeginWindowDrag() const override;

		void SetVSync(bool enabled) override;
		[[nodiscard]] auto IsVSync() const noexcept -> bool override;

		[[nodiscard]] auto GetNativeWindow() const noexcept -> void* override { return m_Window.get(); }

	private:
		static void InitGLFW();
		inline static uint32_t s_WindowCount = 0;

		void Init(const WindowProps& props);
		void CreateNativeWindow(const WindowProps& props);
		void SetGLFWCallbacks();
		void SetWindowCallbacks();
		void SetKeyboardCallbacks();
		void SetMouseCallbacks();
		void Queue(Event event) noexcept;
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
		};

		WindowData m_Data;
		glm::vec2 m_Cursor{};
		std::vector<Event> m_Pending;
		std::vector<Event> m_Events;
		std::exception_ptr m_EventError;
		struct NativeWindowDeleter
		{
			void operator()(GLFWwindow* window) const noexcept;
		};
		std::unique_ptr<GLFWwindow, NativeWindowDeleter> m_Window;
		Scope<GraphicsContext> m_Context;
		mutable bool m_WindowDragActive = false;
		mutable glm::vec2 m_WindowDragStartMouseScreen = {0.0f, 0.0f};
		mutable glm::ivec2 m_WindowDragStartPosition = {0, 0};

		[[nodiscard]] static auto From(GLFWwindow* window) noexcept -> GLFWWindow&;
	};

}
