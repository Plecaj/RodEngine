#include "rdpch.h"

#include "EditorSceneManager.h"

#include "Rod/Project/Project.h"
#include "Rod/Scene/SceneSerializer.h"
#include "Rod/Scripting/ScriptEngine.h"

namespace Rod {

	EditorSceneManager::EditorSceneManager(const Project& project, uint32_t viewportWidth, uint32_t viewportHeight)
	    : m_Project(project), m_ViewportWidth(viewportWidth), m_ViewportHeight(viewportHeight)
	{
		m_EditorScene.ResizeViewport(m_ViewportWidth, m_ViewportHeight);
	}

	EditorSceneManager::~EditorSceneManager()
	{
		Stop();
	}

	void EditorSceneManager::NewScene()
	{
		Stop();
		m_EditorScene = {};
		m_EditorScene.ResizeViewport(m_ViewportWidth, m_ViewportHeight);
		m_ScenePath.clear();
	}

	void EditorSceneManager::OpenScene(const std::filesystem::path& path)
	{
		Scene loadedScene;
		loadedScene.ResizeViewport(m_ViewportWidth, m_ViewportHeight);
		const SceneSerializer serializer(loadedScene, m_Project.GetDirectory());
		serializer.DeserializeText(path.string());

		Stop();
		m_EditorScene = std::move(loadedScene);
		m_ScenePath = path;
	}

	void EditorSceneManager::SaveScene(const std::filesystem::path& path)
	{
		const SceneSerializer serializer(m_EditorScene, m_Project.GetDirectory());
		serializer.SerializeText(path.string());
		m_ScenePath = path;
	}

	void EditorSceneManager::Play()
	{
		if (m_RuntimeScene)
			return;

		auto& runtimeScene = m_RuntimeScene.emplace(m_EditorScene);
		ScriptEngine::OnRuntimeStart(runtimeScene);
	}

	void EditorSceneManager::Stop()
	{
		if (!m_RuntimeScene)
			return;

		ScriptEngine::OnRuntimeStop();
		m_RuntimeScene.reset();
	}

	void EditorSceneManager::ResizeViewport(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
		m_EditorScene.ResizeViewport(width, height);
		if (m_RuntimeScene)
			m_RuntimeScene->ResizeViewport(width, height);
	}

}
