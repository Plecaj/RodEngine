#include "rdpch.h"

#include "DesktopHost.h"

namespace Rod {

	DesktopHost::DesktopHost(const WindowProps& props)
	    : m_Window(Window::Create(props)), m_Input(m_Window->GetCursor()), m_Framebuffer(m_Window->GetFramebufferSize())
	{
		m_Window->SetVSync(false);
	}

	auto DesktopHost::NextFrame() -> std::optional<Frame>
	{
		if (!m_Running)
			return std::nullopt;

		const auto events = m_Window->PollEvents(!IsDrawable());
		for (const Event& event : events)
		{
			if (std::holds_alternative<CloseRequested>(event))
			{
				RequestClose();
				return std::nullopt;
			}
			if (const auto* resize = std::get_if<FramebufferResized>(&event))
				m_Framebuffer = resize->Size;
		}
		m_Input.Update(events, m_Window->GetCursor());

		const bool drawable = IsDrawable();
		Timestep delta;
		if (drawable)
		{
			const auto now = Clock::now();
			if (m_LastFrame)
				delta = Timestep{std::chrono::duration<float>(now - *m_LastFrame).count()};
			m_LastFrame = now;
		}
		else
		{
			m_LastFrame.reset();
		}

		return Frame{.Delta = delta, .Events = events, .Framebuffer = m_Framebuffer, .Drawable = drawable};
	}

	void DesktopHost::Present()
	{
		m_Window->Present();
	}

	auto DesktopHost::IsDrawable() const noexcept -> bool
	{
		return m_Framebuffer.x != 0 && m_Framebuffer.y != 0 && !m_Window->IsMinimized();
	}

}
