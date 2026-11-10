#include "rdpch.h"

#include "Rod/Core/UUID.h"
#include "Rod/Project/Project.h"
#include "Rod/Project/ProjectSerializer.h"

#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <stdexcept>
#include <system_error>

namespace {
	class ProjectSerializationFixture
	{
	public:
		ProjectSerializationFixture()
		{
			do
			{
				m_Directory = std::filesystem::temp_directory_path() /
				              ("rod-project-serialization-" + std::to_string(static_cast<uint64_t>(Rod::UUID{})));
			} while (!std::filesystem::create_directory(m_Directory));
		}
		ProjectSerializationFixture(const ProjectSerializationFixture&) = delete;
		ProjectSerializationFixture& operator=(const ProjectSerializationFixture&) = delete;
		ProjectSerializationFixture(ProjectSerializationFixture&&) = delete;
		ProjectSerializationFixture& operator=(ProjectSerializationFixture&&) = delete;
		~ProjectSerializationFixture()
		{
			std::error_code error;
			std::filesystem::remove_all(m_Directory, error);
		}

	protected:
		[[nodiscard]] std::filesystem::path ManifestPath() const { return m_Directory / "project.rodproj"; }
		void WriteManifest(const std::string& text) const
		{
			std::ofstream file(ManifestPath());
			REQUIRE(file.is_open());
			file << text;
			REQUIRE(file.good());
		}

	private:
		std::filesystem::path m_Directory;
	};
}

TEST_CASE_METHOD(ProjectSerializationFixture, "Project manifests round trip all properties", "[project][serialization]")
{
	const Rod::ProjectProperties properties{
	    .Name = "Game: \"one\"\n# project",
	    .AssetsPath = "Game Assets",
	    .StartupScenePath = "Scenes/Opening Scene.rod",
	    .ScriptsProjectPath = "Scripts/My Game.csproj"};
	const Rod::Project original{ManifestPath(), properties};
	SECTION("new manifest") {}
	SECTION("replace existing manifest")
	{
		WriteManifest("Name: Previous project\n");
	}
	Rod::ProjectSerializer::Serialize(original);

	Rod::Project restored{ManifestPath(), {}};
	Rod::ProjectSerializer::Deserialize(restored);
	REQUIRE(restored.GetProperties().Name == properties.Name);
	REQUIRE(restored.GetAssetsPath() == properties.AssetsPath);
	REQUIRE(restored.GetStartupScenePath() == properties.StartupScenePath);
	REQUIRE(restored.GetScriptProjectPath() == properties.ScriptsProjectPath);
}

TEST_CASE_METHOD(
    ProjectSerializationFixture,
    "Invalid project manifests preserve existing properties",
    "[project][serialization]")
{
	SECTION("missing file") {}
	SECTION("malformed YAML")
	{
		WriteManifest("Name: [unterminated\n");
	}
	SECTION("missing field")
	{
		WriteManifest("Name: New\nAssetsPath: Content\nStartupScenePath: Scenes/New.rod\n");
	}

	Rod::Project project{ManifestPath(), {}};
	const auto original = project.GetProperties();
	REQUIRE_THROWS_AS(Rod::ProjectSerializer::Deserialize(project), std::runtime_error);
	REQUIRE(project.GetProperties().Name == original.Name);
	REQUIRE(project.GetAssetsPath() == original.AssetsPath);
	REQUIRE(project.GetStartupScenePath() == original.StartupScenePath);
	REQUIRE(project.GetScriptProjectPath() == original.ScriptsProjectPath);
}
