#pragma once

#include "Rod/Renderer/Texture.h"

#include <imgui.h>

#include <functional>

namespace Rod {

	class RenderDevice;

	class EditorCommands;
	class Window;

	class TitlebarPanel
	{
	public:
		TitlebarPanel(
		    EditorCommands& commands,
		    const Window& window,
		    const RenderDevice& device,
		    std::function<void()> requestClose);

		void OnImGuiRender() const;

		[[nodiscard]] static constexpr auto GetHeight() noexcept -> uint32_t { return s_Height; }

	private:
		static constexpr uint32_t s_Height = 36;
		static constexpr float s_ButtonWidth = 46.0f;
		void DrawFileMenu() const;
		void DrawButtons() const;

	private:
		EditorCommands& m_Commands;
		const Window& m_Window;
		std::function<void()> m_RequestClose;
		ImVec2 m_ButtonSize;

		Scope<Texture2D> m_Logo;
	};

}
