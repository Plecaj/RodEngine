#pragma once

#include "Rod/Core/InputState.h"
#include "Rod/Core/Timestep.h"
#include "Rod/Core/Window.h"

#include <chrono>
#include <optional>

namespace Rod {

	struct Frame
	{
		Timestep Delta;
		std::span<const Event> Events; // Valid until the next call to NextFrame.
		glm::uvec2 Framebuffer;
		bool Drawable;
	};

	class DesktopHost
	{
	public:
		explicit DesktopHost(const WindowProps& props);
		~DesktopHost() = default;
		DesktopHost(const DesktopHost&) = delete;
		auto operator=(const DesktopHost&) -> DesktopHost& = delete;
		DesktopHost(DesktopHost&&) = delete;
		auto operator=(DesktopHost&&) -> DesktopHost& = delete;

		[[nodiscard]] auto NextFrame() -> std::optional<Frame>;
		void Present();
		void RequestClose() noexcept { m_Running = false; }

		[[nodiscard]] auto GetWindow() const noexcept -> const Window& { return *m_Window; }
		[[nodiscard]] auto GetInput() const noexcept -> const InputState& { return m_Input; }

	private:
		using Clock = std::chrono::steady_clock;

		[[nodiscard]] auto IsDrawable() const noexcept -> bool;

		Scope<Window> m_Window;
		InputState m_Input;
		glm::uvec2 m_Framebuffer;
		std::optional<Clock::time_point> m_LastFrame;
		bool m_Running = true;
	};

}
