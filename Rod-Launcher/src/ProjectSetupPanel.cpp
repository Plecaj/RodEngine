#include "rdpch.h"

#include "ProjectSetupPanel.h"

#include "Rod/Project/ProjectSerializer.h"

#include "imgui.h"

#include <stdexcept>
#include <string_view>

namespace Rod {

	namespace {
		constexpr float g_FormPadding = 32.0f;
		constexpr float g_FooterHeight = 76.0f;
		constexpr float g_FieldSpacing = 12.0f;
		constexpr ImVec2 g_ButtonSize{144.0f, 36.0f};
		constexpr auto g_DefaultName = "Sandbox";
		constexpr auto g_DefaultAssetDirectory = "Assets";

		bool HasText(const char* value)
		{
			return std::string_view(value).find_first_not_of(" \t\r\n") != std::string_view::npos;
		}
	}

	void ProjectSetupPanel::Open()
	{
		m_Open = true;
		m_Step = Step::Directory;
		m_Error.clear();
	}

	std::optional<Project> ProjectSetupPanel::OnImGuiRender()
	{
		constexpr auto titles = std::to_array<const char*>({"Directory", "Project details", "Project files"});
		const auto stepIndex = static_cast<size_t>(m_Step);
		const float width = ImGui::GetWindowWidth() - g_FormPadding - g_FormPadding;
		const float height = ImGui::GetWindowHeight() - g_FormPadding - g_FooterHeight;
		ImGui::SetCursorPos(ImVec2{g_FormPadding, g_FormPadding});
		if (ImGui::BeginChild("ProjectSetup", ImVec2{width, height}))
		{
			ImGui::TextUnformatted("New Project");
			ImGui::TextDisabled("Step %zu of %zu - %s", stepIndex + 1, titles.size(), titles[stepIndex]);
			ImGui::Dummy(ImVec2{0.0f, g_FieldSpacing});
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
		ImGui::EndChild();
		return DrawNavigation();
	}

	void ProjectSetupPanel::DrawField(const char* label, const char* hint, std::span<char> value)
	{
		ImGui::PushID(label);
		ImGui::TextUnformatted(label);
		ImGui::SetNextItemWidth(-1.0f);
		ImGui::InputTextWithHint("##Value", hint, value.data(), value.size());
		ImGui::Dummy(ImVec2{0.0f, g_FieldSpacing});
		ImGui::PopID();
	}

	void ProjectSetupPanel::DrawDirectory()
	{
		DrawField("Directory", "D:/Projects", m_Directory);
		ImGui::TextWrapped("Choose the parent directory. A folder with the project name will be created inside it.");
	}

	void ProjectSetupPanel::DrawDetails()
	{
		DrawField("Project name", g_DefaultName, m_Name);
		DrawField("Asset directory", g_DefaultAssetDirectory, m_AssetDirectory);
		ImGui::TextDisabled("The asset directory is relative to the project folder.");
		ImGui::TextWrapped("Project folder: %s", ProjectFolder().c_str());
	}

	void ProjectSetupPanel::DrawFiles()
	{
		const std::string defaultScene = AssetDirectory() + "/Scenes/Main.rod";
		const std::string defaultScriptProject = AssetDirectory() + "/Scripts/" + ProjectName() + ".csproj";
		DrawField("Start scene", defaultScene.c_str(), m_StartScene);
		DrawField("Script project", defaultScriptProject.c_str(), m_ScriptProject);
		ImGui::TextDisabled("Paths are relative to the project folder.");
		ImGui::TextWrapped("Project file: %s/%s.rdproj", ProjectFolder().c_str(), ProjectName().c_str());
		ImGui::TextWrapped(
		    "The manifest and asset directory will be created. Scene and script files must be provided separately.");
		if (!m_Error.empty())
			ImGui::TextWrapped("%s", m_Error.c_str());
	}

	std::optional<Project> ProjectSetupPanel::DrawNavigation()
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

	Project ProjectSetupPanel::CreateProject() const
	{
		const std::filesystem::path name = ProjectName();
		if (name.has_parent_path() || name.has_root_path() || name == "." || name == ".." || name.filename().empty())
			throw std::runtime_error("Project name must be a single folder name.");

		const auto assets = AssetDirectory();
		ProjectProperties properties{
		    .Name = ProjectName(),
		    .AssetsPath = assets,
		    .StartupScenePath = HasText(m_StartScene.data()) ? m_StartScene.data() : assets + "/Scenes/Main.rod",
		    .ScriptsProjectPath = HasText(m_ScriptProject.data()) ? m_ScriptProject.data()
		                                                          : assets + "/Scripts/" + ProjectName() + ".csproj"};
		for (const auto& path : {properties.AssetsPath, properties.StartupScenePath, properties.ScriptsProjectPath})
		{
			if (path.has_root_path() || path.empty() || std::ranges::find(path, "..") != path.end())
				throw std::runtime_error("Project paths must be relative and remain inside the project folder.");
		}

		const auto directory = std::filesystem::absolute(ProjectFolder()).lexically_normal();
		if (std::filesystem::exists(directory))
			throw std::runtime_error("Project folder already exists. Choose another name or open its manifest.");
		std::filesystem::create_directories(directory.parent_path());
		if (!std::filesystem::create_directory(directory))
			throw std::runtime_error("Project folder already exists.");

		Project project{directory / (ProjectName() + ".rdproj"), std::move(properties)};
		std::filesystem::create_directories(project.GetAssetsPath());
		ProjectSerializer::Serialize(project);
		return project;
	}

	bool ProjectSetupPanel::CanContinue() const
	{
		return m_Step != Step::Directory || HasText(m_Directory.data());
	}

	std::string ProjectSetupPanel::ProjectName() const
	{
		return HasText(m_Name.data()) ? m_Name.data() : g_DefaultName;
	}

	std::string ProjectSetupPanel::AssetDirectory() const
	{
		return HasText(m_AssetDirectory.data()) ? m_AssetDirectory.data() : g_DefaultAssetDirectory;
	}

	std::string ProjectSetupPanel::ProjectFolder() const
	{
		std::string directory = m_Directory.data();
		if (!directory.empty() && directory.back() != '/' && directory.back() != '\\')
			directory += '/';
		return directory + ProjectName();
	}

}
