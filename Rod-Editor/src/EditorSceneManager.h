#pragma once

#include "Rod/Scene/Scene.h"
#include "SceneState.h"

#include <cstdint>
#include <filesystem>

namespace Rod {

	class Project;

	class EditorSceneManager
	{
	public:
		EditorSceneManager(const Project& project, uint32_t viewportWidth, uint32_t viewportHeight);
		~EditorSceneManager();
		EditorSceneManager(const EditorSceneManager&) = delete;
		EditorSceneManager& operator=(const EditorSceneManager&) = delete;
		EditorSceneManager(EditorSceneManager&&) = delete;
		EditorSceneManager& operator=(EditorSceneManager&&) = delete;

		void NewScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveScene(const std::filesystem::path& path);
		void Play();
		void Stop();
		void ResizeViewport(uint32_t width, uint32_t height);

		[[nodiscard]] constexpr Scene& GetActiveScene() noexcept
		{
			return m_State == SceneState::Play ? m_RuntimeScene : m_EditorScene;
		}
		[[nodiscard]] constexpr Scene& GetEditorScene() noexcept { return m_EditorScene; }
		[[nodiscard]] constexpr SceneState GetState() const noexcept { return m_State; }
		[[nodiscard]] constexpr const std::filesystem::path& GetScenePath() const noexcept { return m_ScenePath; }

	private:
		const Project& m_Project;
		Scene m_EditorScene;
		Scene m_RuntimeScene;
		SceneState m_State = SceneState::Edit;
		std::filesystem::path m_ScenePath;
		uint32_t m_ViewportWidth;
		uint32_t m_ViewportHeight;
	};

}
