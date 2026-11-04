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

		void OnImGuiRender();

		void SetNewSceneCallback(std::function<void()> callback) { m_NewScene = std::move(callback); }
		void SetOpenSceneCallback(std::function<void()> callback) { m_OpenScene = std::move(callback); }
		void SetSaveSceneCallback(std::function<void()> callback) { m_SaveScene = std::move(callback); }
		void SetSaveSceneAsCallback(std::function<void()> callback) { m_SaveSceneAs = std::move(callback); }
		void SetExportGameCallback(std::function<void()> callback) { m_ExportGame = std::move(callback); }

		[[nodiscard]] uint32_t GetHeight() const { return HEIGHT; }

	private:
		enum class ButtonIcon : uint8_t
		{
			Minimize,
			Maximize,
			Restore,
			Close
		};
		static constexpr uint32_t HEIGHT = 36;
		static constexpr float BUTTON_WIDTH = 46.0f;
		static bool DrawTitlebarButton(const char* id, ButtonIcon icon, const ImVec2& size);
		static void DrawButtonIcon(ImDrawList& drawList, ButtonIcon icon, const ImVec2& center);
		static void RunCallback(const std::function<void()>& callback);
		void DrawFileMenu();
		void DrawButtons();

	private:
		ImVec2 m_ButtonSize;

		Scope<Texture2D> m_Logo;
		std::function<void()> m_NewScene, m_OpenScene, m_SaveScene, m_SaveSceneAs, m_ExportGame;
	};

}
