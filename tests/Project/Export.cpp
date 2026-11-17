#include "rdpch.h"

#include "Rod/Core/UUID.h"
#include "Rod/Project/Project.h"
#include "Rod/Project/ProjectExporter.h"
#include "Rod/Project/ProjectSerializer.h"
#include "Rod/Scene/Entity.h"
#include "Rod/Scene/Scene.h"
#include "Rod/Scripting/ScriptBuilder.h"
#include "Rod/Utils/FileUtils.h"

#include <catch2/catch_test_macros.hpp>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/yaml.h> // IWYU pragma: keep

#include <stdexcept>
#include <system_error>

namespace {
	class ExportFixture
	{
	public:
		ExportFixture()
		    : Root(
		          std::filesystem::temp_directory_path() /
		          ("rod-export-" + std::to_string(static_cast<uint64_t>(Rod::UUID{})))),
		      Runtime(Root / "Engine/Rod-Runtime"), Source(
		                                                Root / "Source/Project.rodproj",
		                                                {.Name = "Test Game",
		                                                 .AssetsPath = "Content",
		                                                 .StartupScenePath = "Content/Scenes/Main.rod",
		                                                 .ScriptsProjectPath = "Content/Scripts/Game.csproj",
		                                                 .ShadersPath = Root / "Shaders"})
		{
			Rod::WriteTextFile(Runtime, "runtime");
			Rod::WriteTextFile(Runtime.parent_path() / "nethost.dll", "native dependency");
			const auto core = Runtime.parent_path() / "RodAssets/Scripts/Core";
			Rod::WriteTextFile(core / "Rod.ScriptCore.dll", "managed core");
			Rod::WriteTextFile(core / "Rod.ScriptCore.runtimeconfig.json", "{}");
			for (const auto* shader : {"Texture.glsl", "3D.glsl", "Shadow.glsl"})
				Rod::WriteTextFile(Source.GetShadersPath() / shader, "custom shader");
			Rod::WriteTextFile(Source.GetShadersPath() / "Cached/old.spv", "cache");
			Rod::WriteTextFile(
			    Source.GetStartupScenePath(),
			    "Scene: Saved\nEntities:\n  - Entity: 1\n    SpriteRendererComponent:\n"
			    "      Texture: Content/Textures/marker.png\n");
			Rod::WriteTextFile(Source.GetAssetsPath() / "Textures/marker.png", "texture");
			Rod::WriteTextFile(Source.GetScriptProjectPath(), "project");
			Rod::WriteTextFile(Source.GetScriptProjectPath().parent_path() / "Source/Game.cs", "source");
			Rod::WriteTextFile(Source.GetScriptProjectPath().parent_path() / "obj/project.json", "intermediate");
			Rod::WriteTextFile(Source.GetScriptProjectPath().parent_path() / "Cache/old.dll", "cache");
			Rod::WriteTextFile(Rod::ScriptBuilder::GetAssemblyPath(Source.GetScriptProjectPath()), "game assembly");
			Rod::WriteTextFile(
			    Rod::ScriptBuilder::GetAssemblyPath(Source.GetScriptProjectPath()).parent_path() / "Dependency.dll",
			    "dependency");
		}
		ExportFixture(const ExportFixture&) = delete;
		ExportFixture& operator=(const ExportFixture&) = delete;
		ExportFixture(ExportFixture&&) = delete;
		ExportFixture& operator=(ExportFixture&&) = delete;
		~ExportFixture()
		{
			std::error_code error;
			std::filesystem::remove_all(Root, error);
		}

		const std::filesystem::path Root;
		const std::filesystem::path Runtime;
		const Rod::Project Source;
	};
}

TEST_CASE_METHOD(ExportFixture, "Game package paths remain valid after relocation", "[project][export]")
{
	const auto output = Root / "Export";
	Rod::ProjectExporter::Export(Source, Runtime, output);
	const auto relocated = Root / "Moved Game";
	std::filesystem::rename(output, relocated);
	Rod::Project packaged{relocated / Rod::ProjectExporter::s_ManifestFilename, {}};
	Rod::ProjectSerializer::Deserialize(packaged);
	REQUIRE(packaged.GetProperties().AssetsPath == "Content");
	REQUIRE(packaged.GetProperties().ShadersPath == "RodAssets/Shaders");
	REQUIRE(std::filesystem::is_regular_file(relocated / "RodGame"));
	REQUIRE(std::filesystem::is_regular_file(relocated / "nethost.dll"));
	REQUIRE(std::filesystem::is_regular_file(packaged.GetShadersPath() / "Texture.glsl"));
	REQUIRE(std::filesystem::is_regular_file(relocated / "RodAssets/Scripts/Core/Rod.ScriptCore.dll"));
	REQUIRE(std::filesystem::is_regular_file(Rod::ScriptBuilder::GetAssemblyPath(packaged.GetScriptProjectPath())));
	REQUIRE(
	    std::filesystem::is_regular_file(
	        Rod::ScriptBuilder::GetAssemblyPath(packaged.GetScriptProjectPath()).parent_path() / "Dependency.dll"));
	const auto scene = YAML::LoadFile(packaged.GetStartupScenePath().string());
	const auto texture = scene["Entities"][0]["SpriteRendererComponent"]["Texture"].as<std::string>();
	REQUIRE(std::filesystem::is_regular_file(packaged.GetDirectory() / texture));
}

TEST_CASE_METHOD(ExportFixture, "Failed re-export preserves the previous package", "[project][export]")
{
	const auto output = Root / "Export";
	Rod::ProjectExporter::Export(Source, Runtime, output);
	Rod::WriteTextFile(output / "old-package.txt", "keep on failure");
	SECTION("missing required shader")
	{
		std::filesystem::remove(Source.GetShadersPath() / "Shadow.glsl");
	}
	SECTION("missing runtime executable")
	{
		std::filesystem::remove(Runtime);
	}
	SECTION("missing managed runtime configuration")
	{
		std::filesystem::remove(Runtime.parent_path() / "RodAssets/Scripts/Core/Rod.ScriptCore.runtimeconfig.json");
	}
	SECTION("missing game assembly")
	{
		std::filesystem::remove(Rod::ScriptBuilder::GetAssemblyPath(Source.GetScriptProjectPath()));
	}
	SECTION("missing scene asset")
	{
		std::filesystem::remove(Source.GetAssetsPath() / "Textures/marker.png");
	}
	REQUIRE_THROWS_AS(Rod::ProjectExporter::Export(Source, Runtime, output), std::runtime_error);
	REQUIRE(std::filesystem::is_regular_file(output / "old-package.txt"));
	REQUIRE(std::ranges::none_of(std::filesystem::directory_iterator(Root), [](const auto& entry) {
		return entry.path().filename().string().starts_with("Export.tmp-");
	}));
}

TEST_CASE_METHOD(ExportFixture, "Successful re-export replaces the package with the unsaved scene", "[project][export]")
{
	const auto output = Root / "Export";
	Rod::ProjectExporter::Export(Source, Runtime, output);
	Rod::WriteTextFile(output / "old-package.txt", "replace on success");
	Rod::Scene scene;
	const auto entity = scene.CreateEntity("Unsaved entity");
	Rod::ProjectExporter::Export(Source, Runtime, output, &scene);
	REQUIRE_FALSE(std::filesystem::exists(output / "old-package.txt"));
	const auto saved = YAML::LoadFile((output / "Content/Scenes/Startup.rod").string());
	REQUIRE(saved["Entities"][0]["TagComponent"]["Tag"].as<std::string>() == entity.GetName());
	REQUIRE(std::ranges::none_of(std::filesystem::directory_iterator(Root), [](const auto& entry) {
		return entry.path().filename().string().starts_with("Export.tmp-");
	}));
}

TEST_CASE_METHOD(ExportFixture, "Export rejects destinations that could overwrite source files", "[project][export]")
{
	for (const auto& output : {Root, Source.GetDirectory(), Source.GetAssetsPath(), Source.GetAssetsPath() / "Export"})
		REQUIRE_THROWS_AS(Rod::ProjectExporter::Export(Source, Runtime, output), std::runtime_error);
	REQUIRE(std::filesystem::is_regular_file(Source.GetStartupScenePath()));
	REQUIRE(std::filesystem::is_regular_file(Runtime));
}
