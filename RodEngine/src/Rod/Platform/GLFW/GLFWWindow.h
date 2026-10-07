#pragma once

#include "Rod/Core/Window.h"
#include "Rod/Renderer/GraphicsContext.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace Rod {

	class GLFWWindow : public Window
	{
	public:
		GLFWWindow(const WindowProps& props);
		~GLFWWindow() override;

		void OnUpdate() override;

		inline unsigned int GetWidth() const override { return m_Data.Width; }
		inline unsigned int GetHeight() const override { return m_Data.Height; }

		void Minimize() const override;
		void Maximize() const override;
		void Restore() const override;
		void BeginWindowDrag() const override;

		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		inline void* GetNativeWindow() const override { return m_Window; };
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
		mutable glm::vec2 m_WindowDragStartMouseScreen = { 0.0f, 0.0f };
		mutable glm::ivec2 m_WindowDragStartPosition = { 0, 0 };

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;

		static WindowData& GetWindowData(GLFWwindow* window);

	};

}
