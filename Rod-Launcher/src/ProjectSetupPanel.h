#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string>

namespace Rod {

	class ProjectSetupPanel
	{
	public:
		void Open();
		[[nodiscard]] bool IsOpen() const noexcept { return m_Open; }
		void OnImGuiRender();

	private:
		enum class Step : uint8_t
		{
			Directory,
			Details,
			Files
		};

		static void DrawField(const char* label, const char* hint, std::span<char> value);
		void DrawDirectory();
		void DrawDetails();
		void DrawFiles();
		void DrawNavigation();
		[[nodiscard]] bool CanContinue() const;
		[[nodiscard]] std::string ProjectName() const;
		[[nodiscard]] std::string AssetDirectory() const;
		[[nodiscard]] std::string ProjectFolder() const;

		static constexpr size_t FIELD_CAPACITY = 1024;
		using Field = std::array<char, FIELD_CAPACITY>;
		Field m_Directory{};
		Field m_Name{};
		Field m_AssetDirectory{};
		Field m_StartScene{};
		Field m_ScriptProject{};
		Step m_Step = Step::Directory;
		bool m_Open = false;
		bool m_ShowCreationNotice = false;
	};

}
