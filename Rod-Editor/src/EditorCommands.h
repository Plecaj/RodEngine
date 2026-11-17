#pragma once

#include <filesystem>

namespace Rod {

	class Project;
	class EditorSceneManager;
	struct EditorInteractionState;

	class EditorCommands
	{
	public:
		EditorCommands(
		    const Project& project,
		    EditorSceneManager& scenes,
		    EditorInteractionState& interaction) noexcept;

		void NewScene() const;
		void OpenScene() const;
		void OpenScene(const std::filesystem::path& path) const;
		void SaveScene() const;
		void SaveSceneAs() const;
		void ExportGame() const;
		void Play() const;
		void Stop() const;

	private:
		void ClearEntityReferences() const noexcept;

		const Project& m_Project;
		EditorSceneManager& m_Scenes;
		EditorInteractionState& m_Interaction;
	};

}
