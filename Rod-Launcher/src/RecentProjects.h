#pragma once

#include "Rod/Project/Project.h"

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace Rod {

	struct RecentProject
	{
		std::string Name;
		std::filesystem::path Path;
		std::string LastOpened;
	};

	class RecentProjects
	{
	public:
		void Load(const std::filesystem::path& path);
		void Save(const std::filesystem::path& path) const;
		void Record(const Project& project);
		[[nodiscard]] const std::vector<RecentProject>& GetEntries() const { return m_Entries; }

	private:
		static constexpr std::size_t MAX_PROJECTS = 10;
		std::vector<RecentProject> m_Entries;
	};

}
