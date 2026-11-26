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
		[[nodiscard]] auto IsOpen() const noexcept -> bool { return m_Open; }
		[[nodiscard]] auto OnImGuiRender() -> std::optional<Project>;

	private:
		enum class Step : uint8_t
		{
			Directory,
			Details,
			Files,
		};

		[[nodiscard]] static auto DrawPathPicker(const char* label, const std::filesystem::path& value) -> bool;
		void PickProjectPath(
		    std::filesystem::path& value,
		    const char* filter = nullptr,
		    const std::filesystem::path& extension = {});
		void ResetDefaultPaths();
		void DrawDirectory();
		void DrawDetails();
		void DrawFiles();
		[[nodiscard]] auto DrawNavigation() -> std::optional<Project>;
		[[nodiscard]] auto CreateProject() const -> Project;
		[[nodiscard]] auto CanContinue() const noexcept -> bool;
		[[nodiscard]] auto ProjectName() const -> std::string;

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
