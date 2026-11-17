#pragma once

#include "Rod/Events/Event.h"
#include "Rod/Events/KeyEvent.h"
#include "Rod/Renderer/Texture.h"

#include <imgui.h>

#include <functional>

namespace Rod {

	class TitlebarPanel
	{
	public:
		TitlebarPanel();

		void OnImGuiRender() const;

		void SetNewSceneCallback(std::function<void()> callback) noexcept { m_NewScene = std::move(callback); }
		void SetOpenSceneCallback(std::function<void()> callback) noexcept { m_OpenScene = std::move(callback); }
		void SetSaveSceneCallback(std::function<void()> callback) noexcept { m_SaveScene = std::move(callback); }
		void SetSaveSceneAsCallback(std::function<void()> callback) noexcept { m_SaveSceneAs = std::move(callback); }
		void SetExportGameCallback(std::function<void()> callback) noexcept { m_ExportGame = std::move(callback); }

		[[nodiscard]] static constexpr uint32_t GetHeight() noexcept { return s_Height; }

	private:
		static constexpr uint32_t s_Height = 36;
		static constexpr float s_ButtonWidth = 46.0f;
		void DrawFileMenu() const;
		void DrawButtons() const;

	private:
		ImVec2 m_ButtonSize;

		Scope<Texture2D> m_Logo;
		std::function<void()> m_NewScene, m_OpenScene, m_SaveScene, m_SaveSceneAs, m_ExportGame;
	};

}
