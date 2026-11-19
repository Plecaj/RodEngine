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
		static constexpr uint32_t s_DefaultWidth = 1600;
		static constexpr uint32_t s_DefaultHeight = 900;

		std::string Title;
		unsigned int Width;
		unsigned int Height;

		std::string TaskbarIconFilepath;
		bool UsesCustomUi;
		bool NativeTitlebar;
		bool Resizable = true;
		bool Centered = false;

		explicit WindowProps(
		    std::string title = "Rod Engine",
		    uint32_t width = s_DefaultWidth,
		    uint32_t height = s_DefaultHeight,
		    std::string taskbarIconFilepath = "",
		    bool usesCustomUi = false) noexcept
		    : Title(std::move(title)), Width(width), Height(height),
		      TaskbarIconFilepath(std::move(taskbarIconFilepath)), UsesCustomUi(usesCustomUi),
		      NativeTitlebar(!usesCustomUi)
		{}
	};

	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		Window() = default;
		virtual ~Window() = default;
		Window(const Window&) = delete;
		auto operator=(const Window&) -> Window& = delete;
		Window(Window&&) = delete;
		auto operator=(Window&&) -> Window& = delete;

		virtual void OnUpdate() = 0;

		[[nodiscard]] virtual auto GetWidth() const noexcept -> unsigned int = 0;
		[[nodiscard]] virtual auto GetHeight() const noexcept -> unsigned int = 0;

		virtual void Minimize() const = 0;
		virtual void Maximize() const = 0;
		[[nodiscard]] virtual auto IsMinimized() const noexcept -> bool = 0;
		[[nodiscard]] virtual auto IsMaximized() const noexcept -> bool = 0;
		virtual void Restore() const = 0;
		virtual void BeginWindowDrag() const = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		[[nodiscard]] virtual auto IsVSync() const noexcept -> bool = 0;

		[[nodiscard]] virtual auto GetNativeWindow() const noexcept -> void* = 0;

		[[nodiscard]] static auto Create(const WindowProps& props = WindowProps()) -> Scope<Window>;
	};

}
