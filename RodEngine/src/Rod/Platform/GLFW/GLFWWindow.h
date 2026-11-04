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
		virtual ~GLFWWindow() override;
		GLFWWindow(const GLFWWindow&) = delete;
		GLFWWindow& operator=(const GLFWWindow&) = delete;
		GLFWWindow(GLFWWindow&&) = delete;
		GLFWWindow& operator=(GLFWWindow&&) = delete;

		virtual void OnUpdate() override;

		[[nodiscard]] virtual unsigned int GetWidth() const noexcept override { return m_Data.Width; }
		[[nodiscard]] virtual unsigned int GetHeight() const noexcept override { return m_Data.Height; }

		virtual void Minimize() const override;
		virtual void Maximize() const override;
		virtual void Restore() const override;
		virtual void BeginWindowDrag() const override;

		virtual void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		virtual void SetVSync(bool enabled) override;
		[[nodiscard]] virtual bool IsVSync() const noexcept override;

		[[nodiscard]] virtual void* GetNativeWindow() const noexcept override { return m_Window; };

	private:
		void Init(const WindowProps& props);
		void Shutdown();
		void InitGLFW();
		void CreateNativeWindow(const WindowProps& props);
		void SetGLFWCallbacks();
		void SetWindowCallbacks();
		void SetKeyboardCallbacks();
		void SetMouseCallbacks();
		void SetTaskbarIcon(const std::string& iconFilepath) const;
		void ResetWindowDrag() const;
		void UpdateWindowDrag() const;
		glm::vec2 GetMouseScreenPosition() const;

	private:
		GLFWwindow* m_Window = nullptr;
		Scope<GraphicsContext> m_Context;
		mutable bool m_WindowDragActive = false;
		mutable glm::vec2 m_WindowDragStartMouseScreen = {0.0f, 0.0f};
		mutable glm::ivec2 m_WindowDragStartPosition = {0, 0};

		struct WindowData
		{
			std::string Title;
			unsigned int Width = 0, Height = 0;
			bool VSync = false;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;

		static WindowData& GetWindowData(GLFWwindow* window);
	};

}
