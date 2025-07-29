#pragma once

#include "rdpch.h"

#include "Rod/Events/Event.h"

#include <glm/glm.hpp>

namespace Rod {

	struct WindowProps
	{
		std::string Title;
		unsigned int Width;
		unsigned int Height;

		// _IsEditor is internal, shoudnt be changed ever to true by user
		bool _IsEditor;

		WindowProps(const std::string& title = "Rod Engine",
			uint32_t width = 1600,
			uint32_t height = 900,
			bool _Editor = false)
			: Title(title), Width(width), Height(height), _IsEditor(_Editor)
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

		virtual void Minimalize() const = 0;
		virtual void Maximalize() const = 0;
		virtual void Restore() const = 0;
		virtual void BeginWindowDrag() const = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;

		static Window* Create(const WindowProps& props = WindowProps());
	};

}