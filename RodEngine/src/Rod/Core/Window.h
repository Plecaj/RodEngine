#pragma once

#include "Rod/Core/Core.h"
#include "Rod/Events/Event.h"

#include <cstdint>
#include <functional>
#include <string>
#include <utility>

namespace Rod {

	struct WindowProps
	{
		static constexpr uint32_t DEFAULT_WIDTH = 1600;
		static constexpr uint32_t DEFAULT_HEIGHT = 900;

		std::string Title;
		unsigned int Width;
		unsigned int Height;

		std::string TaskbarIconFilepath;
		bool IsEditor;

		explicit WindowProps(
		    std::string title = "Rod Engine",
		    uint32_t width = DEFAULT_WIDTH,
		    uint32_t height = DEFAULT_HEIGHT,
		    std::string taskbarIconFilepath = "",
		    bool isEditor = false)
		    : Title(std::move(title)), Width(width), Height(height),
		      TaskbarIconFilepath(std::move(taskbarIconFilepath)), IsEditor(isEditor)
		{}
	};

	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		Window() = default;
		virtual ~Window() = default;
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(Window&&) = delete;

		virtual void OnUpdate() = 0;

		[[nodiscard]] virtual unsigned int GetWidth() const noexcept = 0;
		[[nodiscard]] virtual unsigned int GetHeight() const noexcept = 0;

		virtual void Minimize() const = 0;
		virtual void Maximize() const = 0;
		virtual void Restore() const = 0;
		virtual void BeginWindowDrag() const = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		[[nodiscard]] virtual bool IsVSync() const noexcept = 0;

		[[nodiscard]] virtual void* GetNativeWindow() const noexcept = 0;

		[[nodiscard]] static Scope<Window> Create(const WindowProps& props = WindowProps());
	};

}
