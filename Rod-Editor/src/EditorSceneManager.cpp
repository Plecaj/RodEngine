#include "rdpch.h"

#include "EditorSceneManager.h"

#include "Rod/Project/Project.h"
#include "Rod/Scene/SceneSerializer.h"

namespace Rod {

	EditorSceneManager::EditorSceneManager(const Project& project, uint32_t viewportWidth, uint32_t viewportHeight)
	    : m_Project(project), m_ViewportWidth(viewportWidth), m_ViewportHeight(viewportHeight)
	{
		m_EditorScene.OnViewportResize(m_ViewportWidth, m_ViewportHeight);
	}

	EditorSceneManager::~EditorSceneManager()
	{
		Stop();
	}

	void EditorSceneManager::NewScene()
	{
		Stop();
		m_EditorScene = {};
		m_EditorScene.OnViewportResize(m_ViewportWidth, m_ViewportHeight);
		m_ScenePath.clear();
	}

	void EditorSceneManager::OpenScene(const std::filesystem::path& path)
	{
		Scene loadedScene;
		loadedScene.OnViewportResize(m_ViewportWidth, m_ViewportHeight);
		const SceneSerializer serializer(loadedScene, m_Project.GetDirectory());
		serializer.DeserializeText(path.string());

		Stop();
		m_EditorScene = std::move(loadedScene);
		m_ScenePath = path;
	}

	void EditorSceneManager::SaveScene(const std::filesystem::path& path)
	{
		const SceneSerializer serializer(GetActiveScene(), m_Project.GetDirectory());
		serializer.SerializeText(path.string());
		m_ScenePath = path;
	}

	void EditorSceneManager::Play()
	{
		if (m_State != SceneState::Edit)
			return;

		m_RuntimeScene = m_EditorScene;
		m_RuntimeScene.OnViewportResize(m_ViewportWidth, m_ViewportHeight);
		m_State = SceneState::Play;
		m_RuntimeScene.OnRuntimeStart();
	}

	void EditorSceneManager::Stop()
	{
		if (m_State != SceneState::Play)
			return;

		m_RuntimeScene.OnRuntimeStop();
		m_State = SceneState::Edit;
	}

	void EditorSceneManager::ResizeViewport(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
		GetActiveScene().OnViewportResize(width, height);
	}

}
