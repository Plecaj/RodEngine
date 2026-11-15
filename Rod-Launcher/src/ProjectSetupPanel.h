#pragma once

#include "Rod/Project/Project.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

namespace Rod {

	class ProjectSetupPanel
	{
	public:
		void Open();
		[[nodiscard]] bool IsOpen() const noexcept { return m_Open; }
		std::optional<Project> OnImGuiRender();

	private:
		enum class Step : uint8_t
		{
			Directory,
			Details,
			Files
		};

		static bool DrawPathPicker(const char* label, const std::filesystem::path& value);
		void PickProjectPath(
		    std::filesystem::path& value,
		    const char* filter = nullptr,
		    const char* extension = nullptr);
		void ResetDefaultPaths();
		void DrawDirectory();
		void DrawDetails();
		void DrawFiles();
		std::optional<Project> DrawNavigation();
		[[nodiscard]] Project CreateProject() const;
		[[nodiscard]] bool CanContinue() const;
		[[nodiscard]] std::string ProjectName() const;

		static constexpr size_t FIELD_CAPACITY = 1024;
		std::array<char, FIELD_CAPACITY> m_Name{};
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
