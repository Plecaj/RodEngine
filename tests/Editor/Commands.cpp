#include "rdpch.h"

#include "../../Rod-Editor/src/EditorCommands.h"
#include "../../Rod-Editor/src/EditorInteractionState.h"
#include "../../Rod-Editor/src/EditorSceneManager.h"
#include "../TemporaryDirectory.h"
#include "Rod/Core/Log.h"
#include "Rod/Project/Project.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Scene commands clear stale entity references", "[editor][commands]")
{
	const Rod::Tests::TemporaryDirectory directory;
	const Rod::Project project{directory.GetPath() / "test.rodproj", {}};
	Rod::EditorSceneManager scenes{project, 1, 1};
	const auto entity = scenes.GetActiveScene().CreateEntity("Selected");
	Rod::EditorInteractionState interaction{.SelectedEntity = entity, .HoveredEntity = entity};
	const Rod::EditorCommands commands{project, scenes, interaction, {}};

	SECTION("new scene")
	{
		commands.NewScene();
	}
	SECTION("open scene")
	{
		const auto path = directory.GetPath() / "scene.rod";
		scenes.SaveScene(path);
		commands.OpenScene(path);
		REQUIRE(scenes.GetActiveScene().FindEntityByName("Selected"));
	}

	REQUIRE(interaction.SelectedEntity == Rod::Entity{});
	REQUIRE(interaction.HoveredEntity == Rod::Entity{});
}

TEST_CASE("Failed open commands preserve the scene and selection", "[editor][commands]")
{
	if (!Rod::Log::GetCoreLogger())
		Rod::Log::Init();
	const Rod::Tests::TemporaryDirectory directory;
	const Rod::Project project{directory.GetPath() / "test.rodproj", {}};
	Rod::EditorSceneManager scenes{project, 1, 1};
	const auto entity = scenes.GetActiveScene().CreateEntity("Selected");
	Rod::EditorInteractionState interaction{.SelectedEntity = entity, .HoveredEntity = entity};
	const Rod::EditorCommands commands{project, scenes, interaction, {}};

	commands.OpenScene(directory.GetPath() / "missing.rod");

	REQUIRE(scenes.GetActiveScene().FindEntityByName("Selected") == entity);
	REQUIRE(interaction.SelectedEntity == entity);
	REQUIRE(interaction.HoveredEntity == entity);
}
