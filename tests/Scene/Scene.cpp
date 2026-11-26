#include "rdpch.h"

#include "Rod/Scene/Scene.h"

#include "Rod/Scene/Components.h"
#include "Rod/Scene/Entity.h"

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>

TEST_CASE("Scene creates entities with default components", "[scene]")
{
	Rod::Scene scene;
	auto entity = scene.CreateEntity();

	REQUIRE(entity);
	REQUIRE(entity.HasComponent<Rod::IDComponent>());
	REQUIRE(entity.HasComponent<Rod::TagComponent>());
	REQUIRE(entity.HasComponent<Rod::TransformComponent>());
	REQUIRE(entity.GetName() == "Entity");
	const auto& transform = entity.GetComponent<Rod::TransformComponent>();
	REQUIRE(transform.Translation == glm::vec3(0.0f));
	REQUIRE(transform.Rotation == glm::vec3(0.0f));
	REQUIRE(transform.Scale == glm::vec3(1.0f));
}

TEST_CASE("Scene finds entities by UUID and name", "[scene]")
{
	Rod::Scene scene;
	auto player = scene.CreateEntityWithUUID(Rod::UUID{1001}, "Player");
	scene.CreateEntityWithUUID(Rod::UUID{1002}, "Enemy");

	REQUIRE(player.GetUUID() == Rod::UUID{1001});
	REQUIRE(scene.FindEntityByUUID(Rod::UUID{1001}) == player);
	REQUIRE(scene.FindEntityByName("Player") == player);
	REQUIRE(!scene.FindEntityByUUID(Rod::UUID{9999}));
	REQUIRE(!scene.FindEntityByName("Missing"));
}

TEST_CASE("Scene owns entity identity", "[scene]")
{
	Rod::Scene scene;
	const auto player = scene.CreateEntityWithUUID(Rod::UUID{1001}, "Player");
	REQUIRE_THROWS_AS(scene.CreateEntityWithUUID(Rod::UUID{1001}), std::invalid_argument);

	Rod::Scene other;
	const auto foreign = other.CreateEntity("Foreign");
	scene.DestroyEntity(foreign);
	REQUIRE(player);
	REQUIRE(foreign);
}

TEST_CASE("Entity component lookup handles optional and removed components", "[scene]")
{
	Rod::Scene scene;
	auto entity = scene.CreateEntity();
	REQUIRE_FALSE(entity.TryGetComponent<Rod::MeshComponent>());
	auto& mesh = entity.AddComponent<Rod::MeshComponent>();
	REQUIRE(entity.TryGetComponent<Rod::MeshComponent>() == &mesh);
	entity.RemoveComponent<Rod::MeshComponent>();
	REQUIRE_FALSE(entity.TryGetComponent<Rod::MeshComponent>());
	scene.DestroyEntity(entity);
	REQUIRE_FALSE(entity.TryGetComponent<Rod::TransformComponent>());
}

TEST_CASE("Scene equality compares component data", "[scene]")
{
	Rod::Scene original;
	original.ResizeViewport(800, 400);
	auto entity = original.CreateEntityWithUUID(Rod::UUID{1001}, "Player");
	entity.AddComponent<Rod::SpriteRendererComponent>();
	entity.AddComponent<Rod::MeshComponent>();
	entity.AddComponent<Rod::DirectionalLightComponent>();
	entity.AddComponent<Rod::CameraComponent>();
	auto& script = entity.AddComponent<Rod::ScriptComponent>("Game.Player");
	script.Fields["Speed"] = {
	    .Field = {.Name = "Speed", .TypeName = "System.Single", .Type = Rod::ScriptFieldType::Float},
	    .Value = "3.0"};
	Rod::Scene copy(original);
	REQUIRE(original == copy);
	REQUIRE(copy == original);

	auto copiedEntity = copy.FindEntityByUUID(Rod::UUID{1001});
	SECTION("tag")
	{
		copiedEntity.GetComponent<Rod::TagComponent>().Tag = "Changed";
	}
	SECTION("transform")
	{
		copiedEntity.GetComponent<Rod::TransformComponent>().Translation.x = 1.0f;
	}
	SECTION("sprite")
	{
		copiedEntity.GetComponent<Rod::SpriteRendererComponent>().TilingFactor = 2.0f;
	}
	SECTION("mesh material")
	{
		copiedEntity.GetComponent<Rod::MeshComponent>().Material.SetMetallic(0.75f);
	}
	SECTION("light")
	{
		copiedEntity.GetComponent<Rod::DirectionalLightComponent>().Intensity = 2.0f;
	}
	SECTION("camera")
	{
		copiedEntity.GetComponent<Rod::CameraComponent>().Primary = false;
	}
	SECTION("script field")
	{
		copiedEntity.GetComponent<Rod::ScriptComponent>().Fields.at("Speed").Value = "4.0";
	}
	SECTION("component presence")
	{
		copiedEntity.RemoveComponent<Rod::MeshComponent>();
	}
	SECTION("viewport")
	{
		copy.ResizeViewport(400, 400);
	}
	REQUIRE_FALSE(original == copy);
	REQUIRE_FALSE(copy == original);
}

TEST_CASE("Scene destroys only the requested entity", "[scene]")
{
	Rod::Scene scene;
	const auto player = scene.CreateEntityWithUUID(Rod::UUID{1001}, "Player");
	auto enemy = scene.CreateEntityWithUUID(Rod::UUID{1002}, "Enemy");

	scene.DestroyEntity(player);

	REQUIRE(!player);
	REQUIRE(!scene.FindEntityByUUID(Rod::UUID{1001}));
	REQUIRE(!scene.FindEntityByName("Player"));
	REQUIRE(scene.FindEntityByUUID(Rod::UUID{1002}) == enemy);
}

TEST_CASE("Scene selects the primary camera", "[scene]")
{
	Rod::Scene scene;
	scene.ResizeViewport(1920, 1080);
	REQUIRE(!scene.FindPrimaryCamera());

	auto minimap = scene.CreateEntity("Minimap");
	minimap.AddComponent<Rod::CameraComponent>().Primary = false;
	REQUIRE(!scene.FindPrimaryCamera());

	auto camera = scene.CreateEntity("Camera");
	camera.AddComponent<Rod::CameraComponent>();
	REQUIRE(scene.FindPrimaryCamera() == camera);
	auto spare = scene.CreateEntity("Spare");
	REQUIRE_FALSE(spare.AddComponent<Rod::CameraComponent>().Primary);
	scene.SetCameraPrimary(spare, true);
	REQUIRE(scene.FindPrimaryCamera() == spare);
	REQUIRE_FALSE(camera.GetComponent<Rod::CameraComponent>().Primary);

	scene.DestroyEntity(spare);
	REQUIRE(!scene.FindPrimaryCamera());
}

namespace {
	class SceneCopyFixture
	{
	public:
		SceneCopyFixture() = default;
		~SceneCopyFixture() = default;
		SceneCopyFixture(const SceneCopyFixture&) = delete;
		auto operator=(const SceneCopyFixture&) -> SceneCopyFixture& = delete;
		SceneCopyFixture(SceneCopyFixture&&) = delete;
		auto operator=(SceneCopyFixture&&) -> SceneCopyFixture& = delete;

		static constexpr glm::vec3 s_Translation{1.0f, 2.0f, 3.0f};
		Rod::Scene Source = CreateSource();
		Rod::Scene Copy{Source};
		Rod::Entity Player = Source.FindEntityByUUID(Rod::UUID{1001});
		Rod::Entity CopiedPlayer = Copy.FindEntityByUUID(Rod::UUID{1001});

	private:
		static auto CreateSource() -> Rod::Scene
		{
			Rod::Scene scene;
			auto player = scene.CreateEntityWithUUID(Rod::UUID{1001}, "Player");
			player.GetComponent<Rod::TransformComponent>().Translation = s_Translation;
			player.AddComponent<Rod::SpriteRendererComponent>().TilingFactor = 2.0f;
			return scene;
		}
	};
}

TEST_CASE("Scene copy preserves entity identity and components", "[scene]")
{
	SceneCopyFixture fixture;
	REQUIRE(fixture.CopiedPlayer);
	REQUIRE(fixture.CopiedPlayer.GetName() == "Player");
	REQUIRE(fixture.CopiedPlayer.HasComponent<Rod::TransformComponent>());
	REQUIRE(fixture.CopiedPlayer.HasComponent<Rod::SpriteRendererComponent>());
	REQUIRE(
	    fixture.CopiedPlayer.GetComponent<Rod::TransformComponent>().Translation == SceneCopyFixture::s_Translation);
	REQUIRE(fixture.CopiedPlayer.GetComponent<Rod::SpriteRendererComponent>().TilingFactor == 2.0f);
}

TEST_CASE("Scene copies own independent component data", "[scene]")
{
	SceneCopyFixture fixture;
	fixture.CopiedPlayer.GetComponent<Rod::TransformComponent>().Translation.x = 10.0f;
	fixture.CopiedPlayer.GetComponent<Rod::SpriteRendererComponent>().TilingFactor = 5.0f;
	REQUIRE(fixture.Player.GetComponent<Rod::TransformComponent>().Translation.x == SceneCopyFixture::s_Translation.x);
	REQUIRE(fixture.Player.GetComponent<Rod::SpriteRendererComponent>().TilingFactor == 2.0f);
}

TEST_CASE("Destroying a copied entity preserves the original", "[scene]")
{
	SceneCopyFixture fixture;
	fixture.Copy.DestroyEntity(fixture.CopiedPlayer);
	REQUIRE(fixture.Source.FindEntityByUUID(Rod::UUID{1001}) == fixture.Player);
}

TEST_CASE("Scene assignment replaces entities with independent copies", "[scene]")
{
	Rod::Scene source;
	auto player = source.CreateEntityWithUUID(Rod::UUID{1001}, "Player");
	player.AddComponent<Rod::SpriteRendererComponent>();
	Rod::Scene destination;
	destination.CreateEntityWithUUID(Rod::UUID{1002}, "Old");

	destination = source;
	REQUIRE(!destination.FindEntityByName("Old"));
	auto copy = destination.FindEntityByUUID(player.GetUUID());
	REQUIRE(copy);
	copy.GetComponent<Rod::SpriteRendererComponent>().TilingFactor = 2.0f;
	REQUIRE(player.GetComponent<Rod::SpriteRendererComponent>().TilingFactor == 1.0f);

	const Rod::Scene& alias = destination;
	destination = alias;
	REQUIRE(destination.FindEntityByName("Player"));
	destination = Rod::Scene{};
	REQUIRE(!destination.FindEntityByName("Player"));
}

TEST_CASE("Scene moves transfer entities and replace destination contents", "[scene]")
{
	Rod::Scene source;
	auto entity = source.CreateEntityWithUUID(Rod::UUID{1001}, "Player");
	entity.AddComponent<Rod::ScriptComponent>("Game.Player");

	Rod::Scene moved(std::move(source));
	REQUIRE(moved.FindEntityByUUID(Rod::UUID{1001}));

	Rod::Scene destination;
	destination.CreateEntity("Old");
	destination = std::move(moved);
	REQUIRE_FALSE(destination.FindEntityByName("Old"));
	const auto player = destination.FindEntityByUUID(Rod::UUID{1001});
	REQUIRE(player);
	REQUIRE(player.GetName() == "Player");
	REQUIRE(player.GetComponent<Rod::ScriptComponent>().ClassName == "Game.Player");
}
