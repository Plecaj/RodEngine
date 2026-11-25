#include "rdpch.h"

#include "../TemporaryDirectory.h"
#include "Rod/Renderer/RenderDevice.h"
#include "Rod/Scene/Components.h"
#include "Rod/Scene/Entity.h"
#include "Rod/Scene/Scene.h"
#include "Rod/Scene/SceneSerializer.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <yaml-cpp/yaml.h> // IWYU pragma: keep

#include <fstream>
#include <stdexcept>

namespace {
	class SerializationFixture
	{
	public:
		SerializationFixture()
		{
			if (!Rod::Log::GetCoreLogger())
				Rod::Log::Init();
		}

	protected:
		[[nodiscard]] auto Filepath() const -> std::string { return (m_Directory.GetPath() / "scene.rod").string(); }
		void WriteScene(const std::string& text) const
		{
			std::ofstream file(Filepath());
			REQUIRE(file.is_open());
			file << text;
			REQUIRE(file.good());
		}

	private:
		Rod::Tests::TemporaryDirectory m_Directory;
	};
}

TEST_CASE_METHOD(SerializationFixture, "Scene round-trips over an existing file", "[scene][serialization]")
{
	WriteScene(std::string(4096, 'x'));
	Rod::Scene original;
	original.ResizeViewport(1920, 1080);
	original.CreateEntityWithUUID(Rod::UUID{1001});
	auto entity = original.CreateEntityWithUUID(Rod::UUID{1002}, "Player: \"one\"\n# tag");
	auto& transform = entity.GetComponent<Rod::TransformComponent>();
	transform.Translation = {1.25f, -2.5f, 3.75f};
	transform.Rotation = {-0.25f, 0.5f, 0.75f};
	transform.Scale = {2.0f, 0.5f, -1.0f};
	entity.AddComponent<Rod::CameraComponent>();
	entity.AddComponent<Rod::SpriteRendererComponent>();
	entity.AddComponent<Rod::MeshComponent>();
	entity.AddComponent<Rod::DirectionalLightComponent>();
	entity.AddComponent<Rod::ScriptComponent>("Game.Player");
	Rod::SceneSerializer(original).SerializeText(Filepath());

	Rod::Scene restored;
	restored.ResizeViewport(1920, 1080);
	Rod::SceneSerializer(restored).DeserializeText(Filepath(), Rod::RenderDevice{});
	REQUIRE(restored == original);
}

TEST_CASE_METHOD(SerializationFixture, "Empty scenes load", "[scene][serialization]")
{
	WriteScene("Scene: Empty\nEntities: []\n");
	Rod::Scene scene;
	Rod::SceneSerializer(scene).DeserializeText(Filepath(), Rod::RenderDevice{});
	REQUIRE(scene == Rod::Scene{});
}

TEST_CASE_METHOD(SerializationFixture, "Scene loading takes the first marked camera", "[scene][serialization]")
{
	constexpr uint64_t firstID = 1001;
	constexpr uint64_t secondID = 1002;
	const auto camera = YAML::Load(R"(
Camera:
  ProjectionType: 1
  PerspectiveFOV: 0.8
  PerspectiveNear: 0.1
  PerspectiveFar: 100
  OrthographicSize: 10
  OrthographicNear: -1
  OrthographicFar: 1
Primary: true
FixedAspectRatio: false)");
	YAML::Node sceneFile;
	sceneFile["Scene"] = "Two cameras";
	for (const auto id : {firstID, secondID})
	{
		YAML::Node entity;
		entity["Entity"] = id;
		entity["CameraComponent"] = camera;
		sceneFile["Entities"].push_back(entity);
	}
	WriteScene(YAML::Dump(sceneFile));

	Rod::Scene scene;
	Rod::SceneSerializer(scene).DeserializeText(Filepath(), Rod::RenderDevice{});
	const auto first = scene.FindEntityByUUID(Rod::UUID{firstID});
	const auto second = scene.FindEntityByUUID(Rod::UUID{secondID});
	REQUIRE(first);
	REQUIRE(second);
	REQUIRE(scene.FindPrimaryCamera() == first);
	REQUIRE_FALSE(second.GetComponent<Rod::CameraComponent>().Primary);
}

TEST_CASE_METHOD(
    SerializationFixture,
    "Scene asset inspection reads references without loading assets",
    "[scene][serialization]")
{
	WriteScene(R"(
Scene: Assets
Entities:
  - Entity: 1
    SpriteRendererComponent: {Texture: 'Assets\ship.png'}
    MeshComponent: {Path: None}
  - Entity: 2
    MeshComponent: {Path: Assets/ship.glb}
)");
	const auto paths = Rod::SceneSerializer::ReadAssetReferences(Filepath());
	REQUIRE(paths == std::vector<std::filesystem::path>{"Assets/ship.png", "Assets/ship.glb"});
}

TEST_CASE_METHOD(SerializationFixture, "Invalid scene files preserve the scene", "[scene][serialization]")
{
	SECTION("missing file")
	{}
	SECTION("malformed YAML")
	{
		WriteScene("Scene: [unterminated\n");
	}
	SECTION("missing scene header")
	{
		WriteScene("Entities: []\n");
	}
	SECTION("invalid component after a valid entity")
	{
		WriteScene(R"(Scene: Invalid
Entities:
  - Entity: 1001
  - Entity: 1002
    TransformComponent:
      Translation: [1, 2]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
)");
	}
	SECTION("duplicate entity UUID")
	{
		WriteScene("Scene: Invalid\nEntities:\n  - Entity: 1001\n  - Entity: 1001\n");
	}

	Rod::Scene scene;
	scene.CreateEntityWithUUID(Rod::UUID{42}, "Existing");
	const Rod::Scene original = scene;
	REQUIRE_THROWS_AS(Rod::SceneSerializer(scene).DeserializeText(Filepath(), Rod::RenderDevice{}), std::runtime_error);
	REQUIRE(scene == original);
}

TEST_CASE_METHOD(
    SerializationFixture,
    "Missing project-relative assets roll back scene loading",
    "[scene][serialization][project]")
{
	const auto projectDirectory = std::filesystem::path(Filepath()).parent_path() / "My Project";
	std::filesystem::path assetPath;
	std::string component;
	SECTION("texture")
	{
		assetPath = "Content/Missing.png";
		component = "    SpriteRendererComponent:\n      Color: [1, 1, 1, 1]\n      TilingFactor: 1\n      Texture: ";
	}
	SECTION("mesh")
	{
		assetPath = "Content/Missing.glb";
		component = "    MeshComponent:\n      Path: ";
	}
	WriteScene(
	    "Scene: Missing asset\nEntities:\n  - Entity: 1001\n  - Entity: 1002\n" + component +
	    assetPath.generic_string() + "\n");

	Rod::Scene scene;
	scene.CreateEntityWithUUID(Rod::UUID{42}, "Existing");
	const Rod::Scene original = scene;
	const auto expectedPath = (projectDirectory / assetPath).lexically_normal();
	REQUIRE_THROWS_WITH(
	    Rod::SceneSerializer(scene, projectDirectory).DeserializeText(Filepath(), Rod::RenderDevice{}),
	    Catch::Matchers::ContainsSubstring(expectedPath.string()));
	REQUIRE(scene == original);
}
