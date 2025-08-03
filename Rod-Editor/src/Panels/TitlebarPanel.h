#pragma once

#include <imgui.h>

#include "Rod/Events/Event.h"
#include "Rod/Events/KeyEvent.h"

#include "Rod/Renderer/Texture.h"

#include <functional>

namespace Rod {

	class TitlebarPanel {
	public:
		TitlebarPanel();

		void OnImGuiRender();

		void SetNewSceneCallback(std::function<void()> callback) { m_NewScene = std::move(callback); }
		void SetOpenSceneCallback(std::function<void()> callback) { m_OpenScene = std::move(callback); }
		void SetSaveSceneCallback(std::function<void()> callback) { m_SaveScene = std::move(callback); }
		void SetSaveSceneAsCallback(std::function<void()> callback) { m_SaveSceneAs = std::move(callback); }
	private:
		void DrawFileMenu();
		void HandleDrag();
		void DrawButtons();
	private:
		uint32_t m_Width = 0, m_Height = 24;
		ImVec2 m_ButtonSize;

		Ref<Texture2D> m_Logo;
		std::function<void()> m_NewScene, m_OpenScene, m_SaveScene, m_SaveSceneAs;
	};

}