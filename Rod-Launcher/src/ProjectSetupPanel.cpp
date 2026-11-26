#include "rdpch.h"

#include "ProjectSetupPanel.h"

#include "Rod/Project/ProjectInitializer.h"
#include "Rod/Project/ProjectSerializer.h"
#include "Rod/Utils/PlatformUtils.h"

#include "imgui.h"

#include <stdexcept>
#include <string_view>

namespace Rod {

	namespace {
		constexpr float g_FormPadding = 32.0f;
		constexpr float g_FooterHeight = 76.0f;
		constexpr float g_FieldSpacing = 12.0f;
		constexpr ImVec2 g_ButtonSize{144.0f, 36.0f};

		[[nodiscard]] constexpr auto HasText(const char* value) noexcept -> bool
		{
			return std::string_view(value).find_first_not_of(" \t\r\n") != std::string_view::npos;
		}
	}

	void ProjectSetupPanel::Open()
	{
		m_Open = true;
		m_Step = Step::Directory;
		m_Name.fill('\0');
		m_Directory.clear();
		ResetDefaultPaths();
		m_Error.clear();
	}

	void ProjectSetupPanel::ResetDefaultPaths()
	{
		const ProjectProperties defaults;
		m_AssetDirectory = defaults.AssetsPath;
		m_StartScene = defaults.StartupScenePath;
		m_ScriptProject = defaults.ScriptsProjectPath;
		m_ShaderDirectory = defaults.ShadersPath;
	}

	auto ProjectSetupPanel::OnImGuiRender() -> std::optional<Project>
	{
		constexpr auto titles = std::to_array<const char*>({"Directory", "Project details", "Project files"});
		const auto stepIndex = static_cast<size_t>(m_Step);
		const float width = ImGui::GetWindowWidth() - g_FormPadding - g_FormPadding;
		const float height = ImGui::GetWindowHeight() - g_FormPadding - g_FooterHeight;
		ImGui::SetCursorPos(ImVec2{g_FormPadding, g_FormPadding});
		if (ImGui::BeginChild("ProjectSetup", ImVec2{width, height}))
		{
			ImGui::TextUnformatted("New Project");
			ImGui::TextDisabled("Step %zu of %zu - %s", stepIndex + 1, titles.size(), titles.at(stepIndex));
			ImGui::Dummy(ImVec2{0.0f, g_FieldSpacing});
			try
			{
				switch (m_Step)
				{
					case Step::Directory:
						DrawDirectory();
						break;
					case Step::Details:
						DrawDetails();
						break;
					case Step::Files:
						DrawFiles();
						break;
				}
			}
			catch (const std::exception& exception)
			{
				m_Error = exception.what();
			}
			if (!m_Error.empty())
				ImGui::TextWrapped("%s", m_Error.c_str());
		}
		ImGui::EndChild();
		return DrawNavigation();
	}

	auto ProjectSetupPanel::DrawPathPicker(const char* label, const std::filesystem::path& value) -> bool
	{
		ImGui::PushID(label);
		ImGui::TextUnformatted(label);
		const bool browse = ImGui::Button("Browse...", g_ButtonSize);
		ImGui::SameLine();
		ImGui::TextWrapped("%s", value.empty() ? "No selection" : value.string().c_str());
		ImGui::Dummy(ImVec2{0.0f, g_FieldSpacing});
		ImGui::PopID();
		return browse;
	}

	void ProjectSetupPanel::PickProjectPath(
	    std::filesystem::path& value,
	    const char* filter,
	    const std::filesystem::path& extension)
	{
		const auto selected = filter ? m_FileDialogs.OpenFile(filter, m_Directory.string())
		                             : m_FileDialogs.SelectFolder(m_Directory.string());
		if (selected.empty())
			return;
		if (filter ? !std::filesystem::is_regular_file(selected) : !std::filesystem::is_directory(selected))
			throw std::runtime_error(filter ? "Choose an existing file." : "Choose an existing folder.");
		const auto relative = std::filesystem::relative(std::filesystem::canonical(selected), m_Directory);
		if (relative.empty() || relative.has_root_path() || std::ranges::find(relative, "..") != relative.end())
			throw std::runtime_error("Choose a path inside the project folder.");
		if (!extension.empty() && relative.extension() != extension)
			throw std::runtime_error("Choose a " + extension.string() + " file.");
		value = relative;
		m_Error.clear();
	}

	void ProjectSetupPanel::DrawDirectory()
	{
		if (DrawPathPicker("Project folder", m_Directory))
		{
			const auto selected = m_FileDialogs.SelectFolder(m_Directory.string());
			if (!selected.empty())
			{
				if (!std::filesystem::is_directory(selected))
					throw std::runtime_error("Choose an existing project folder.");
				const auto directory = std::filesystem::canonical(selected);
				if (directory != m_Directory)
				{
					m_Directory = directory;
					ResetDefaultPaths();
				}
				m_Error.clear();
			}
		}
		ImGui::TextWrapped(
		    "Choose or create the project folder in the system dialog. The manifest will be saved there.");
	}

	void ProjectSetupPanel::DrawDetails()
	{
		ImGui::TextUnformatted("Project name");
		ImGui::SetNextItemWidth(-1.0f);
		ImGui::InputTextWithHint(
		    "##ProjectName",
		    m_Directory.filename().string().c_str(),
		    m_Name.data(),
		    m_Name.size());
		ImGui::Dummy(ImVec2{0.0f, g_FieldSpacing});
		if (DrawPathPicker("Asset directory", m_AssetDirectory))
		{
			const auto previous = m_AssetDirectory;
			PickProjectPath(m_AssetDirectory);
			if (m_StartScene == previous / "Scenes/Main.rod")
				m_StartScene = m_AssetDirectory / "Scenes/Main.rod";
			if (m_ScriptProject == previous / "Scripts/Game.csproj")
				m_ScriptProject = m_AssetDirectory / "Scripts/Game.csproj";
		}
		if (DrawPathPicker("Engine shader directory", m_ShaderDirectory))
		{
			const auto selected = m_FileDialogs.SelectFolder(m_ShaderDirectory.string());
			if (!selected.empty())
			{
				if (!std::filesystem::is_directory(selected))
					throw std::runtime_error("Choose an existing shader folder.");
				m_ShaderDirectory = std::filesystem::canonical(selected);
				m_Error.clear();
			}
		}
		ImGui::TextWrapped("Project folder: %s", m_Directory.string().c_str());
	}

	void ProjectSetupPanel::DrawFiles()
	{
		if (DrawPathPicker("Start scene", m_StartScene))
			PickProjectPath(m_StartScene, "Rod Scene (*.rod)\0*.rod\0", ".rod");
		if (DrawPathPicker("Script project", m_ScriptProject))
			PickProjectPath(m_ScriptProject, "C# Project (*.csproj)\0*.csproj\0", ".csproj");
		ImGui::TextWrapped("Default scene and script files will be created if missing. Browse to use existing files.");
		ImGui::TextWrapped("Project file: %s", (m_Directory / (ProjectName() + ".rodproj")).string().c_str());
	}

	auto ProjectSetupPanel::DrawNavigation() -> std::optional<Project>
	{
		constexpr float bottomPadding = 20.0f;
		const float top = ImGui::GetWindowHeight() - bottomPadding - g_ButtonSize.y;
		ImGui::SetCursorPos(ImVec2{g_FormPadding, top});
		if (ImGui::Button("Cancel", g_ButtonSize))
		{
			m_Open = false;
			return std::nullopt;
		}
		if (m_Step != Step::Directory)
		{
			ImGui::SameLine();
			if (ImGui::Button("Back", g_ButtonSize))
			{
				m_Step = m_Step == Step::Files ? Step::Details : Step::Directory;
				m_Error.clear();
			}
		}
		ImGui::SetCursorPos(ImVec2{ImGui::GetWindowWidth() - g_FormPadding - g_ButtonSize.x, top});
		ImGui::BeginDisabled(!CanContinue());
		std::optional<Project> created;
		if (ImGui::Button(m_Step == Step::Files ? "Create Project" : "Next", g_ButtonSize))
		{
			if (m_Step == Step::Files)
			{
				try
				{
					created = CreateProject();
					m_Open = false;
					m_Error.clear();
				}
				catch (const std::exception& exception)
				{
					m_Error = exception.what();
				}
			}
			else
				m_Step = m_Step == Step::Directory ? Step::Details : Step::Files;
		}
		ImGui::EndDisabled();
		return created;
	}

	auto ProjectSetupPanel::CreateProject() const -> Project
	{
		const std::filesystem::path name = ProjectName();
		if (name.has_parent_path() || name.has_root_path() || name == "." || name == ".." || name.filename().empty())
			throw std::runtime_error("Project name must be a single file name.");

		const auto directory = std::filesystem::canonical(m_Directory);
		for (const auto& entry : std::filesystem::directory_iterator(directory))
		{
			const auto extension = entry.path().extension();
			if (extension == ".rodproj")
				throw std::runtime_error("This folder already has a project manifest. Open it from the launcher.");
		}
		for (const auto* shader : {"Texture.glsl", "3D.glsl", "Shadow.glsl"})
		{
			if (!std::filesystem::is_regular_file(m_ShaderDirectory / shader))
				throw std::runtime_error(std::string("Shader folder is missing ") + shader + ".");
		}

		ProjectProperties properties{
		    .Name = ProjectName(),
		    .AssetsPath = m_AssetDirectory,
		    .StartupScenePath = m_StartScene,
		    .ScriptsProjectPath = m_ScriptProject,
		    .ShadersPath = std::filesystem::canonical(m_ShaderDirectory)};
		Project project{directory / (ProjectName() + ".rodproj"), std::move(properties)};
		ProjectInitializer::CreateDefaultFiles(project);
		ProjectSerializer::Serialize(project);
		return project;
	}

	auto ProjectSetupPanel::CanContinue() const noexcept -> bool
	{
		switch (m_Step)
		{
			case Step::Directory:
				return !m_Directory.empty();
			case Step::Details:
				return !m_AssetDirectory.empty();
			case Step::Files:
				return !m_StartScene.empty() && !m_ScriptProject.empty();
		}
		return false;
	}

	auto ProjectSetupPanel::ProjectName() const -> std::string
	{
		return HasText(m_Name.data()) ? m_Name.data() : m_Directory.filename().string();
	}

}
