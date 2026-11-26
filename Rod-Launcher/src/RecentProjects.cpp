#include "rdpch.h"

#include "RecentProjects.h"

#include "Rod/Utils/FileUtils.h"

#include <yaml-cpp/yaml.h> // IWYU pragma: keep

#include <algorithm>
#include <chrono>
#include <format>
#include <fstream>
#include <stdexcept>
#include <system_error>

namespace Rod {

	namespace {
		auto SameProject(const std::filesystem::path& left, const std::filesystem::path& right) -> bool
		{
			if (left == right)
				return true;
			std::error_code error;
			return std::filesystem::equivalent(left, right, error);
		}
	}

	void RecentProjects::Load(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			m_Entries.clear();
			return;
		}
		std::ifstream file(path);
		if (!file.is_open())
			throw std::runtime_error("Could not read recent projects: " + path.string());
		const auto data = YAML::Load(file);
		if (file.bad() || !data.IsMap() || !data["RecentProjects"].IsSequence())
			throw std::runtime_error("Invalid recent projects manifest: " + path.string());

		std::vector<RecentProject> entries;
		for (const auto& entry : data["RecentProjects"])
		{
			if (!entry.IsMap())
				continue;
			const auto name = entry["Name"];
			const auto manifestPath = entry["Path"];
			const auto lastOpened = entry["LastOpened"];
			if (!name || !name.IsScalar() || !manifestPath || !manifestPath.IsScalar() || !lastOpened ||
			    !lastOpened.IsScalar())
				continue;
			RecentProject project{
			    .Name = name.as<std::string>(),
			    .Path = std::filesystem::path(manifestPath.as<std::string>()).lexically_normal(),
			    .LastOpened = lastOpened.as<std::string>(),
			};
			if (!project.Path.is_absolute() || project.Path.extension() != ".rodproj")
				continue;
			if (std::ranges::any_of(entries, [&](const auto& existing) {
				    return SameProject(existing.Path, project.Path);
			    }))
				continue;
			entries.push_back(std::move(project));
			if (entries.size() == s_MaxProjects)
				break;
		}
		m_Entries = std::move(entries);
	}

	void RecentProjects::Save(const std::filesystem::path& path) const
	{
		YAML::Emitter out;
		out << YAML::BeginMap << YAML::Key << "RecentProjects" << YAML::Value << YAML::BeginSeq;
		for (const auto& project : m_Entries)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Name" << YAML::Value << project.Name;
			out << YAML::Key << "Path" << YAML::Value << project.Path.generic_string();
			out << YAML::Key << "LastOpened" << YAML::Value << project.LastOpened;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq << YAML::EndMap;
		if (!out.good())
			throw std::runtime_error("Could not serialize recent projects: " + out.GetLastError());
		WriteTextFile(path, out.c_str());
	}

	void RecentProjects::Record(const Project& project)
	{
		RecentProject entry{
		    .Name = project.GetProperties().Name,
		    .Path = std::filesystem::canonical(project.GetManifestPath()),
		    .LastOpened = std::format("{:%F}", std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now())),
		};
		std::erase_if(m_Entries, [&](const auto& existing) { return SameProject(existing.Path, entry.Path); });
		m_Entries.insert(m_Entries.begin(), std::move(entry));
		if (m_Entries.size() > s_MaxProjects)
			m_Entries.resize(s_MaxProjects);
	}

}
