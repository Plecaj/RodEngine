#include "rdpch.h"

#include "../../Rod-Launcher/src/RecentProjects.h"

#include "Rod/Core/UUID.h"

#include <catch2/catch_test_macros.hpp>
#include <yaml-cpp/yaml.h>

#include <fstream>
#include <system_error>

namespace {
	class RecentProjectsFixture
	{
	public:
		RecentProjectsFixture()
		{
			do
			{
				m_Directory = std::filesystem::temp_directory_path() /
				              ("rod-recent-projects-" + std::to_string(static_cast<uint64_t>(Rod::UUID{})));
			} while (!std::filesystem::create_directory(m_Directory));
		}
		RecentProjectsFixture(const RecentProjectsFixture&) = delete;
		RecentProjectsFixture& operator=(const RecentProjectsFixture&) = delete;
		RecentProjectsFixture(RecentProjectsFixture&&) = delete;
		RecentProjectsFixture& operator=(RecentProjectsFixture&&) = delete;
		~RecentProjectsFixture()
		{
			std::error_code error;
			std::filesystem::remove_all(m_Directory, error);
		}

	protected:
		[[nodiscard]] std::filesystem::path HistoryPath() const { return m_Directory / "RecentProjects.yaml"; }
		[[nodiscard]] Rod::Project MakeProject(const std::string& name) const
		{
			const auto path = m_Directory / (name + ".rodproj");
			std::ofstream file(path);
			REQUIRE(file.is_open());
			return {path, {.Name = name}};
		}
		void WriteHistory(const std::string& text) const
		{
			std::ofstream file(HistoryPath());
			REQUIRE(file.is_open());
			file << text;
			REQUIRE(file.good());
		}

	private:
		std::filesystem::path m_Directory;
	};
}

TEST_CASE_METHOD(
    RecentProjectsFixture,
    "Recent projects retain the newest ten across reloads",
    "[launcher][recent-projects]")
{
	Rod::RecentProjects history;
	for (int index = 0; index < 12; ++index)
		history.Record(MakeProject("Project " + std::to_string(index)));
	REQUIRE(history.GetEntries().size() == 10);
	REQUIRE(history.GetEntries().front().Name == "Project 11");
	REQUIRE(history.GetEntries().back().Name == "Project 2");
	history.Save(HistoryPath());
	history.Save(HistoryPath());

	Rod::RecentProjects restored;
	restored.Load(HistoryPath());
	REQUIRE(restored.GetEntries().size() == history.GetEntries().size());
	for (std::size_t index = 0; index < history.GetEntries().size(); ++index)
	{
		const auto& original = history.GetEntries()[index];
		const auto& loaded = restored.GetEntries()[index];
		REQUIRE(loaded.Name == original.Name);
		REQUIRE(loaded.Path == original.Path);
		REQUIRE(loaded.LastOpened == original.LastOpened);
		REQUIRE(loaded.LastOpened.size() == 10);
	}
}

TEST_CASE_METHOD(
    RecentProjectsFixture,
    "Reopening a recent project moves and updates its entry",
    "[launcher][recent-projects]")
{
	auto first = MakeProject("First");
	Rod::RecentProjects history;
	history.Record(first);
	history.Record(MakeProject("Second"));
	first.SetProperties({.Name = "Renamed: # project"});
	history.Record(first);
	REQUIRE(history.GetEntries().size() == 2);
	REQUIRE(history.GetEntries().front().Name == first.GetProperties().Name);
	REQUIRE(history.GetEntries().front().Path == std::filesystem::canonical(first.GetManifestPath()));
	REQUIRE(history.GetEntries().back().Name == "Second");
	history.Save(HistoryPath());
	history.Load(HistoryPath());
	REQUIRE(history.GetEntries().front().Name == first.GetProperties().Name);
}

TEST_CASE_METHOD(
    RecentProjectsFixture,
    "Missing and invalid recent histories are handled predictably",
    "[launcher][recent-projects]")
{
	Rod::RecentProjects history;
	history.Load(HistoryPath());
	REQUIRE(history.GetEntries().empty());
	history.Record(MakeProject("Existing"));
	SECTION("missing history resets entries")
	{
		history.Load(HistoryPath());
		REQUIRE(history.GetEntries().empty());
	}
	SECTION("malformed YAML preserves entries")
	{
		WriteHistory("RecentProjects: [unterminated\n");
		REQUIRE_THROWS(history.Load(HistoryPath()));
		REQUIRE(history.GetEntries().front().Name == "Existing");
	}
	SECTION("wrong root type preserves entries")
	{
		WriteHistory("RecentProjects: wrong\n");
		REQUIRE_THROWS(history.Load(HistoryPath()));
		REQUIRE(history.GetEntries().front().Name == "Existing");
	}
}

TEST_CASE_METHOD(
    RecentProjectsFixture,
    "History loading skips invalid and duplicate entries and enforces the limit",
    "[launcher][recent-projects]")
{
	YAML::Emitter out;
	out << YAML::BeginMap << YAML::Key << "RecentProjects" << YAML::Value << YAML::BeginSeq;
	out << "invalid entry";
	out << YAML::BeginMap << YAML::Key << "Name" << YAML::Value << "Missing fields" << YAML::EndMap;
	for (int index = 0; index < 13; ++index)
	{
		const auto project = MakeProject("Project " + std::to_string(index));
		for (int duplicate = 0; duplicate < 2; ++duplicate)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Name" << YAML::Value << project.GetProperties().Name;
			out << YAML::Key << "Path" << YAML::Value << project.GetManifestPath().generic_string();
			out << YAML::Key << "LastOpened" << YAML::Value << "2026-11-16";
			out << YAML::EndMap;
		}
	}
	out << YAML::EndSeq << YAML::EndMap;
	WriteHistory(out.c_str());
	Rod::RecentProjects history;
	history.Load(HistoryPath());
	REQUIRE(history.GetEntries().size() == 10);
	REQUIRE(history.GetEntries().front().Name == "Project 0");
	REQUIRE(history.GetEntries().back().Name == "Project 9");
}
