#pragma once

#include "Rod/Scene/Scene.h"
#include "SceneState.h"

#include <cstdint>
#include <filesystem>
#include <optional>

namespace Rod {

	class Project;
	class RenderDevice;

	class EditorSceneManager
	{
	public:
		EditorSceneManager(
		    const Project& project,
		    const RenderDevice& device,
		    uint32_t viewportWidth,
		    uint32_t viewportHeight);
		~EditorSceneManager();
		EditorSceneManager(const EditorSceneManager&) = delete;
		auto operator=(const EditorSceneManager&) -> EditorSceneManager& = delete;
		EditorSceneManager(EditorSceneManager&&) = delete;
		auto operator=(EditorSceneManager&&) -> EditorSceneManager& = delete;

		void NewScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveScene(const std::filesystem::path& path);
		void Play();
		void Stop();
		void ResizeViewport(uint32_t width, uint32_t height);

		[[nodiscard]] constexpr auto GetActiveScene() noexcept -> Scene&
		{
			return m_RuntimeScene ? *m_RuntimeScene : m_EditorScene;
		}
		[[nodiscard]] constexpr auto GetEditorScene() noexcept -> Scene& { return m_EditorScene; }
		[[nodiscard]] constexpr auto GetState() const noexcept -> SceneState
		{
			return m_RuntimeScene ? SceneState::Play : SceneState::Edit;
		}
		[[nodiscard]] constexpr auto GetScenePath() const noexcept -> const std::filesystem::path&
		{
			return m_ScenePath;
		}

	private:
		const Project& m_Project;
		const RenderDevice& m_Device;
		Scene m_EditorScene;
		std::optional<Scene> m_RuntimeScene;
		std::filesystem::path m_ScenePath;
		uint32_t m_ViewportWidth;
		uint32_t m_ViewportHeight;
	};

}
