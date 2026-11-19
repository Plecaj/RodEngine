#include "rdpch.h"

#include "EditorCommands.h"

#include "EditorInteractionState.h"
#include "EditorSceneManager.h"
#include "Rod/Project/Project.h"
#include "Rod/Project/ProjectExporter.h"
#include "Rod/Scripting/ScriptBuilder.h"

#include <exception>
#include <stdexcept>

namespace Rod {

	EditorCommands::EditorCommands(
	    const Project& project,
	    EditorSceneManager& scenes,
	    EditorInteractionState& interaction,
	    FileDialogs fileDialogs) noexcept
	    : m_Project(project), m_Scenes(scenes), m_Interaction(interaction), m_FileDialogs(fileDialogs)
	{}

	void EditorCommands::NewScene() const
	{
		Stop();
		m_Scenes.NewScene();
		ClearEntityReferences();
	}

	void EditorCommands::ClearEntityReferences() const noexcept
	{
		m_Interaction.SelectedEntity = {};
		m_Interaction.HoveredEntity = {};
	}

	void EditorCommands::OpenScene() const
	{
		const std::string filepath = m_FileDialogs.OpenFile("Rod Scene (*.rod)\0*.rod\0");
		if (!filepath.empty())
			OpenScene(filepath);
	}

	void EditorCommands::OpenScene(const std::filesystem::path& path) const
	{
		try
		{
			const bool wasPlaying = m_Scenes.GetState() == SceneState::Play;
			m_Scenes.OpenScene(path);
			ClearEntityReferences();
			if (wasPlaying)
				m_Interaction.GizmoType = -1;
		}
		catch (const std::exception& exception)
		{
			RD_CORE_ERROR("{}", exception.what());
		}
	}

	void EditorCommands::SaveScene() const
	{
		if (m_Scenes.GetScenePath().empty())
		{
			SaveSceneAs();
			return;
		}

		try
		{
			m_Scenes.SaveScene(m_Scenes.GetScenePath());
		}
		catch (const std::exception& exception)
		{
			RD_CORE_ERROR("{}", exception.what());
		}
	}

	void EditorCommands::SaveSceneAs() const
	{
		const std::string filepath = m_FileDialogs.SaveFile("Rod Scene (*.rod)\0*.rod\0");
		if (!filepath.empty())
		{
			try
			{
				m_Scenes.SaveScene(filepath);
			}
			catch (const std::exception& exception)
			{
				RD_CORE_ERROR("{}", exception.what());
			}
		}
	}

	void EditorCommands::ExportGame() const
	{
		try
		{
			const auto scripts = ScriptBuilder::BuildProject(m_Project.GetScriptProjectPath());
			if (!scripts.Success)
				throw std::runtime_error("Script build failed: " + scripts.Output);
			const auto directory = Platform::GetExecutablePath().parent_path();
			const auto output = directory / "exports/RodGame";
			ProjectExporter::Export(
			    m_Project,
			    directory / "Rod-Runtime" ROD_EXECUTABLE_SUFFIX,
			    output,
			    m_Scenes.GetEditorScene());
			RD_CORE_INFO("Exported game to {}", output.string());
		}
		catch (const std::exception& exception)
		{
			RD_CORE_ERROR("Game export failed: {}", exception.what());
		}
	}

	void EditorCommands::Play() const
	{
		if (m_Scenes.GetState() != SceneState::Edit)
			return;

		m_Scenes.Play();
		ClearEntityReferences();
		m_Interaction.GizmoType = -1;
	}

	void EditorCommands::Stop() const
	{
		if (m_Scenes.GetState() != SceneState::Play)
			return;

		m_Scenes.Stop();
		ClearEntityReferences();
		m_Interaction.GizmoType = -1;
	}

} // namespace Rod
