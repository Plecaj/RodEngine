#include "rdpch.h"

#include "../../Rod-Editor/src/EditorSceneManager.h"
#include "../TemporaryDirectory.h"
#include "Rod/Project/Project.h"
#include "Rod/Renderer/RenderDevice.h"
#include "Rod/Scene/Entity.h" // IWYU pragma: keep

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Editor scenes can be saved, cleared and reopened", "[editor][scene-manager]")
{
	const Rod::Tests::TemporaryDirectory directory;
	const Rod::Project project{directory.GetPath() / "test.rodproj", {}};
	const Rod::RenderDevice device;
	Rod::EditorSceneManager scenes{project, device, 1, 1};
	const auto path = directory.GetPath() / "scene.rod";
	scenes.GetActiveScene().CreateEntity("Saved");
	scenes.SaveScene(path);

	scenes.NewScene();
	REQUIRE(scenes.GetScenePath().empty());
	REQUIRE_FALSE(scenes.GetActiveScene().FindEntityByName("Saved"));

	scenes.OpenScene(path);
	REQUIRE(scenes.GetScenePath() == path);
	REQUIRE(scenes.GetActiveScene().FindEntityByName("Saved"));
}

TEST_CASE("Saving during Play writes the authored scene", "[editor][scene-manager]")
{
	const Rod::Tests::TemporaryDirectory directory;
	const Rod::Project project{directory.GetPath() / "test.rodproj", {}};
	const Rod::RenderDevice device;
	Rod::EditorSceneManager scenes{project, device, 1, 1};
	const auto path = directory.GetPath() / "scene.rod";
	scenes.GetEditorScene().CreateEntity("Authored");
	scenes.Play();
	scenes.GetActiveScene().CreateEntity("Runtime only");
	scenes.SaveScene(path);
	scenes.OpenScene(path);
	REQUIRE(scenes.GetState() == Rod::SceneState::Edit);
	REQUIRE(scenes.GetActiveScene().FindEntityByName("Authored"));
	REQUIRE_FALSE(scenes.GetActiveScene().FindEntityByName("Runtime only"));
}
