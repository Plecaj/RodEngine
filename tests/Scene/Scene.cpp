#include "rdpch.h"

#include "Rod/Scene/Scene.h"

#include "Rod/Scene/Components.h"
#include "Rod/Scene/Entity.h"

#include <catch2/catch_test_macros.hpp>

namespace {
	constexpr uint64_t g_PlayerID = 1001;
	constexpr uint64_t g_EnemyID = 1002;
	constexpr uint64_t g_MissingID = 9999;
}

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
	auto player = scene.CreateEntityWithUUID(Rod::UUID{g_PlayerID}, "Player");
	scene.CreateEntityWithUUID(Rod::UUID{g_EnemyID}, "Enemy");

	REQUIRE(player.GetUUID() == Rod::UUID{g_PlayerID});
	REQUIRE(scene.FindEntityByUUID(Rod::UUID{g_PlayerID}) == player);
	REQUIRE(scene.FindEntityByName("Player") == player);
	REQUIRE(!scene.FindEntityByUUID(Rod::UUID{g_MissingID}));
	REQUIRE(!scene.FindEntityByName("Missing"));
}

TEST_CASE("Scene equality compares component data", "[scene]")
{
	constexpr uint32_t viewportWidth = 800;
	constexpr uint32_t viewportHeight = 400;
	constexpr float changedTilingFactor = 2.0f;
	constexpr float changedMetallic = 0.75f;
	constexpr float changedIntensity = 2.0f;

	Rod::Scene original;
	original.OnViewportResize(viewportWidth, viewportHeight);
	auto entity = original.CreateEntityWithUUID(Rod::UUID{g_PlayerID}, "Player");
	entity.AddComponent<Rod::SpriteRendererComponent>();
	entity.AddComponent<Rod::MeshComponent>();
	entity.AddComponent<Rod::DirectionalLightComponent>();
	entity.AddComponent<Rod::CameraComponent>();
	auto& script = entity.AddComponent<Rod::ScriptComponent>("Game.Player");
	script.Fields["Speed"] = {{"Speed", "System.Single", Rod::ScriptFieldType::Float}, "3.0"};
	Rod::Scene copy(original);
	REQUIRE(original == copy);
	REQUIRE(copy == original);

	auto copiedEntity = copy.FindEntityByUUID(Rod::UUID{g_PlayerID});
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
		copiedEntity.GetComponent<Rod::SpriteRendererComponent>().TilingFactor = changedTilingFactor;
	}
	SECTION("mesh material")
	{
		copiedEntity.GetComponent<Rod::MeshComponent>().Material.SetMetallic(changedMetallic);
	}
	SECTION("light")
	{
		copiedEntity.GetComponent<Rod::DirectionalLightComponent>().Intensity = changedIntensity;
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
		copy.OnViewportResize(viewportHeight, viewportHeight);
	}
	REQUIRE_FALSE(original == copy);
	REQUIRE_FALSE(copy == original);
}

TEST_CASE("Scene destroys only the requested entity", "[scene]")
{
	Rod::Scene scene;
	auto player = scene.CreateEntityWithUUID(Rod::UUID{g_PlayerID}, "Player");
	auto enemy = scene.CreateEntityWithUUID(Rod::UUID{g_EnemyID}, "Enemy");

	scene.DestroyEntity(player);

	REQUIRE(!player);
	REQUIRE(!scene.FindEntityByUUID(Rod::UUID{g_PlayerID}));
	REQUIRE(!scene.FindEntityByName("Player"));
	REQUIRE(scene.FindEntityByUUID(Rod::UUID{g_EnemyID}) == enemy);
}

TEST_CASE("Scene selects the primary camera", "[scene]")
{
	Rod::Scene scene;
	constexpr glm::uvec2 viewportSize{1920, 1080};
	scene.OnViewportResize(viewportSize.x, viewportSize.y);
	REQUIRE(!scene.GetPrimaryCameraEntity());

	auto minimap = scene.CreateEntity("Minimap");
	minimap.AddComponent<Rod::CameraComponent>().Primary = false;
	REQUIRE(!scene.GetPrimaryCameraEntity());

	auto camera = scene.CreateEntity("Camera");
	camera.AddComponent<Rod::CameraComponent>();
	REQUIRE(scene.GetPrimaryCameraEntity() == camera);

	scene.DestroyEntity(camera);
	REQUIRE(!scene.GetPrimaryCameraEntity());
}

namespace {
	class SceneCopyFixture
	{
	public:
		SceneCopyFixture() = default;
		~SceneCopyFixture() = default;
		SceneCopyFixture(const SceneCopyFixture&) = delete;
		SceneCopyFixture& operator=(const SceneCopyFixture&) = delete;
		SceneCopyFixture(SceneCopyFixture&&) = delete;
		SceneCopyFixture& operator=(SceneCopyFixture&&) = delete;

		static constexpr glm::vec3 TRANSLATION{1.0f, 2.0f, 3.0f};
		static constexpr float INITIAL_TILING = 2.0f;
		static constexpr float MODIFIED_TILING = 5.0f;
		static constexpr float MODIFIED_X = 10.0f;
		Rod::Scene Source = CreateSource();
		Rod::Scene Copy{Source};
		Rod::Entity Player = Source.FindEntityByUUID(Rod::UUID{g_PlayerID});
		Rod::Entity CopiedPlayer = Copy.FindEntityByUUID(Rod::UUID{g_PlayerID});

	private:
		static Rod::Scene CreateSource()
		{
			Rod::Scene scene;
			auto player = scene.CreateEntityWithUUID(Rod::UUID{g_PlayerID}, "Player");
			player.GetComponent<Rod::TransformComponent>().Translation = TRANSLATION;
			player.AddComponent<Rod::SpriteRendererComponent>().TilingFactor = INITIAL_TILING;
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
	REQUIRE(fixture.CopiedPlayer.GetComponent<Rod::TransformComponent>().Translation == SceneCopyFixture::TRANSLATION);
	REQUIRE(
	    fixture.CopiedPlayer.GetComponent<Rod::SpriteRendererComponent>().TilingFactor ==
	    SceneCopyFixture::INITIAL_TILING);
}

TEST_CASE("Scene copies own independent component data", "[scene]")
{
	SceneCopyFixture fixture;
	fixture.CopiedPlayer.GetComponent<Rod::TransformComponent>().Translation.x = SceneCopyFixture::MODIFIED_X;
	fixture.CopiedPlayer.GetComponent<Rod::SpriteRendererComponent>().TilingFactor = SceneCopyFixture::MODIFIED_TILING;
	REQUIRE(fixture.Player.GetComponent<Rod::TransformComponent>().Translation.x == SceneCopyFixture::TRANSLATION.x);
	REQUIRE(
	    fixture.Player.GetComponent<Rod::SpriteRendererComponent>().TilingFactor == SceneCopyFixture::INITIAL_TILING);
}

TEST_CASE("Destroying a copied entity preserves the original", "[scene]")
{
	SceneCopyFixture fixture;
	fixture.Copy.DestroyEntity(fixture.CopiedPlayer);
	REQUIRE(fixture.Source.FindEntityByUUID(Rod::UUID{g_PlayerID}) == fixture.Player);
}

TEST_CASE("Scene assignment replaces entities with independent copies", "[scene]")
{
	Rod::Scene source;
	auto player = source.CreateEntityWithUUID(Rod::UUID{g_PlayerID}, "Player");
	player.AddComponent<Rod::SpriteRendererComponent>();
	Rod::Scene destination;
	destination.CreateEntityWithUUID(Rod::UUID{g_EnemyID}, "Old");

	REQUIRE(&(destination = source) == &destination);
	REQUIRE(!destination.FindEntityByName("Old"));
	auto copy = destination.FindEntityByUUID(player.GetUUID());
	REQUIRE(copy);
	copy.GetComponent<Rod::SpriteRendererComponent>().TilingFactor = SceneCopyFixture::INITIAL_TILING;
	REQUIRE(player.GetComponent<Rod::SpriteRendererComponent>().TilingFactor == 1.0f);

	const Rod::Scene& alias = destination;
	destination = alias;
	REQUIRE(destination.FindEntityByName("Player"));
	destination = Rod::Scene{};
	REQUIRE(!destination.FindEntityByName("Player"));
}

namespace {
	class TrackedScript : public Rod::ScriptableEntity
	{
	public:
		static inline int s_Destroyed = 0;

	protected:
		virtual void OnDestroy() override { ++s_Destroyed; }
	};
}

TEST_CASE("Scene moves rebind native scripts and release replaced instances", "[scene]")
{
	TrackedScript::s_Destroyed = 0;
	Rod::Scene source;
	auto entity = source.CreateEntityWithUUID(Rod::UUID{g_PlayerID}, "Player");
	auto& script = entity.AddComponent<Rod::NativeScriptComponent>();
	script.Bind<TrackedScript>();
	script.Instance = new TrackedScript;

	Rod::Scene moved(std::move(source));
	auto movedEntity = moved.FindEntityByName("Player");
	REQUIRE(
	    movedEntity.GetComponent<Rod::NativeScriptComponent>().Instance->GetComponent<Rod::IDComponent>().ID ==
	    Rod::UUID{g_PlayerID});

	Rod::Scene destination;
	auto old = destination.CreateEntity("Old");
	auto& oldScript = old.AddComponent<Rod::NativeScriptComponent>();
	oldScript.Bind<TrackedScript>();
	oldScript.Instance = new TrackedScript;
	destination = std::move(moved);
	REQUIRE(TrackedScript::s_Destroyed == 1);
	auto player = destination.FindEntityByName("Player");
	REQUIRE(
	    player.GetComponent<Rod::NativeScriptComponent>().Instance->GetComponent<Rod::TagComponent>().Tag == "Player");
	destination.DestroyEntity(player);
	constexpr int expectedDestroyCount = 2;
	REQUIRE(TrackedScript::s_Destroyed == expectedDestroyCount);
}
