#include "rdpch.h"

#include "Rod/Core/UUID.h"
#include "Rod/Project/Project.h"
#include "Rod/Project/ProjectInitializer.h"
#include "Rod/Scene/Entity.h"
#include "Rod/Scene/Scene.h"
#include "Rod/Scene/SceneSerializer.h"
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
	Rod::ProjectProperties properties{
	    .Name = "Game: \"one\"\n# project",
	    .AssetsPath = "Game Assets",
	    .StartupScenePath = "Scenes/Opening Scene.rod",
	    .ScriptsProjectPath = "Scripts/My Game.csproj",
	    .ShadersPath = "Custom Engine/Shaders"};
	SECTION("new manifest") {}
	SECTION("replace existing manifest")
	{
		WriteManifest("Name: Previous project\n");
	}
	SECTION("absolute shader directory")
	{
		properties.ShadersPath = ManifestPath().parent_path() / "External Shaders";
	}
	const Rod::Project original{ManifestPath(), properties};
	Rod::ProjectSerializer::Serialize(original);

	Rod::Project restored{ManifestPath(), {}};
	Rod::ProjectSerializer::Deserialize(restored);
	REQUIRE(restored.GetProperties().Name == properties.Name);
	REQUIRE(restored.GetProperties().AssetsPath == properties.AssetsPath);
	REQUIRE(restored.GetProperties().StartupScenePath == properties.StartupScenePath);
	REQUIRE(restored.GetProperties().ScriptsProjectPath == properties.ScriptsProjectPath);
	REQUIRE(restored.GetProperties().ShadersPath == properties.ShadersPath);
	REQUIRE(restored.GetShadersPath() == ManifestPath().parent_path() / properties.ShadersPath);
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

	SECTION("invalid shader path")
	{
		WriteManifest("Name: New\nAssetsPath: Assets\nStartupScenePath: Main.rod\nScriptsProjectPath: Game.csproj\nShadersPath: [bad]\n");
	}
	SECTION("missing shader path")
	{
		WriteManifest("Name: New\nAssetsPath: Assets\nStartupScenePath: Main.rod\nScriptsProjectPath: Game.csproj\n");
	}

	Rod::Project project{ManifestPath(), {}};
	const auto original = project.GetProperties();
	REQUIRE_THROWS_AS(Rod::ProjectSerializer::Deserialize(project), std::runtime_error);
	REQUIRE(project.GetProperties().Name == original.Name);
	REQUIRE(project.GetProperties().AssetsPath == original.AssetsPath);
	REQUIRE(project.GetProperties().StartupScenePath == original.StartupScenePath);
	REQUIRE(project.GetProperties().ScriptsProjectPath == original.ScriptsProjectPath);
	REQUIRE(project.GetProperties().ShadersPath == original.ShadersPath);
}

TEST_CASE_METHOD(ProjectSerializationFixture, "New projects create usable defaults without overwriting files", "[project]")
{
	const Rod::Project project{ManifestPath(), {}};
	const auto root = project.GetDirectory();
	REQUIRE(project.GetAssetsPath() == root / "Assets");
	REQUIRE(project.GetStartupScenePath() == root / "Assets/Scenes/Main.rod");
	REQUIRE(project.GetScriptProjectPath() == root / "Assets/Scripts/Game.csproj");
	REQUIRE(project.GetShadersPath() == std::filesystem::absolute("RodAssets/Shaders").lexically_normal());
	Rod::ProjectInitializer::CreateDefaultFiles(project);
	REQUIRE(std::filesystem::is_directory(project.GetAssetsPath()));
	REQUIRE(std::filesystem::is_regular_file(project.GetStartupScenePath()));
	REQUIRE(std::filesystem::is_regular_file(project.GetScriptProjectPath()));
	Rod::Scene scene;
	Rod::SceneSerializer serializer(scene, project.GetDirectory());
	REQUIRE_NOTHROW(serializer.DeserializeText(project.GetStartupScenePath().string()));
	REQUIRE(static_cast<bool>(scene.GetPrimaryCameraEntity()));
	std::ofstream file(project.GetScriptProjectPath());
	file << "preserve this content";
	file.close();
	Rod::ProjectInitializer::CreateDefaultFiles(project);
	std::ifstream restored(project.GetScriptProjectPath());
	std::string contents;
	std::getline(restored, contents);
	REQUIRE(contents == "preserve this content");
}

TEST_CASE_METHOD(ProjectSerializationFixture, "Missing custom project files are rejected before creating defaults", "[project]")
{
	const Rod::Project project{ManifestPath(), {.ScriptsProjectPath = "Custom/Missing.csproj"}};
	REQUIRE_THROWS_AS(Rod::ProjectInitializer::CreateDefaultFiles(project), std::runtime_error);
	REQUIRE_FALSE(std::filesystem::exists(project.GetAssetsPath()));
}

TEST_CASE_METHOD(ProjectSerializationFixture, "Project defaults cannot escape through a directory symlink", "[project]")
{
	const auto assets = ManifestPath().parent_path() / "Assets";
	std::error_code error;
	std::filesystem::create_directory_symlink(ManifestPath().parent_path().parent_path(), assets, error);
	if (error)
		SKIP("Directory symlinks are unavailable: " << error.message());
	const Rod::Project project{ManifestPath(), {}};
	REQUIRE_THROWS_AS(Rod::ProjectInitializer::CreateDefaultFiles(project), std::runtime_error);
}
