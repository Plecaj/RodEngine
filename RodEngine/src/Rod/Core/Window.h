#pragma once

#include "rdpch.h"

#include "Rod/Core/Core.h"
#include "Rod/Events/Event.h"

#include <glm/glm.hpp>

namespace Rod {

	struct WindowProps
	{
		std::string Title;
		unsigned int Width;
		unsigned int Height;

		std::string TaskbarIconFilepath;
		bool IsEditor;

		WindowProps(const std::string& title = "Rod Engine",
			uint32_t width = 1600,
			uint32_t height = 900,
			std::string taskbarIconFilepath = "",
			bool isEditor = false)
			: Title(title), Width(width), Height(height), TaskbarIconFilepath(taskbarIconFilepath), IsEditor(isEditor)
		{
		}
	};

	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		virtual void Minimize() const = 0;
		virtual void Maximize() const = 0;
		virtual void Restore() const = 0;
		virtual void BeginWindowDrag() const = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;

		static Scope<Window> Create(const WindowProps& props = WindowProps());
	};

}
