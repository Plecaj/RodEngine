#pragma once

#include "Rod/Project/Project.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

namespace Rod {

	class FileDialogs;

	class ProjectSetupPanel
	{
	public:
		explicit ProjectSetupPanel(const FileDialogs& fileDialogs) noexcept : m_FileDialogs(fileDialogs) {}

		void Open();
		[[nodiscard]] bool IsOpen() const noexcept { return m_Open; }
		[[nodiscard]] std::optional<Project> OnImGuiRender();

	private:
		enum class Step : uint8_t
		{
			Directory,
			Details,
			Files
		};

		[[nodiscard]] static bool DrawPathPicker(const char* label, const std::filesystem::path& value);
		void
		PickProjectPath(std::filesystem::path& value, const char* filter = nullptr, const char* extension = nullptr);
		void ResetDefaultPaths();
		void DrawDirectory();
		void DrawDetails();
		void DrawFiles();
		[[nodiscard]] std::optional<Project> DrawNavigation();
		[[nodiscard]] Project CreateProject() const;
		[[nodiscard]] bool CanContinue() const noexcept;
		[[nodiscard]] std::string ProjectName() const;

		static constexpr size_t s_FieldCapacity = 1024;
		const FileDialogs& m_FileDialogs;
		std::array<char, s_FieldCapacity> m_Name{};
		std::filesystem::path m_Directory;
		std::filesystem::path m_AssetDirectory;
		std::filesystem::path m_StartScene;
		std::filesystem::path m_ScriptProject;
		std::filesystem::path m_ShaderDirectory;
		Step m_Step = Step::Directory;
		bool m_Open = false;
		std::string m_Error;
	};

}
