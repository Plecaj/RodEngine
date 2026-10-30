#include "Rod/Scene/Scene.h"
#include "Rod/Scene/Components.h"
#include "Rod/Scene/Entity.h"
#include "rdpch.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Scene creates entities with default components", "[scene]") {
  Rod::Scene scene;
  auto entity = scene.CreateEntity();

  REQUIRE(entity);
  REQUIRE(entity.HasComponent<Rod::IDComponent>());
  REQUIRE(entity.HasComponent<Rod::TagComponent>());
  REQUIRE(entity.HasComponent<Rod::TransformComponent>());
  REQUIRE(entity.GetName() == "Entity");
  const auto &transform = entity.GetComponent<Rod::TransformComponent>();
  REQUIRE(transform.Translation == glm::vec3(0.0f));
  REQUIRE(transform.Rotation == glm::vec3(0.0f));
  REQUIRE(transform.Scale == glm::vec3(1.0f));
}

TEST_CASE("Scene finds entities by UUID and name", "[scene]") {
  Rod::Scene scene;
  auto player = scene.CreateEntityWithUUID(Rod::UUID{1001}, "Player");
  scene.CreateEntityWithUUID(Rod::UUID{1002}, "Enemy");

  REQUIRE(player.GetUUID() == Rod::UUID{1001});
  REQUIRE(scene.FindEntityByUUID(Rod::UUID{1001}) == player);
  REQUIRE(scene.FindEntityByName("Player") == player);
  REQUIRE_FALSE(scene.FindEntityByUUID(Rod::UUID{9999}));
  REQUIRE_FALSE(scene.FindEntityByName("Missing"));
}

TEST_CASE("Scene destroys only the requested entity", "[scene]") {
  Rod::Scene scene;
  auto player = scene.CreateEntityWithUUID(Rod::UUID{1001}, "Player");
  auto enemy = scene.CreateEntityWithUUID(Rod::UUID{1002}, "Enemy");

  scene.DestroyEntity(player);

  REQUIRE_FALSE(player);
  REQUIRE_FALSE(scene.FindEntityByUUID(Rod::UUID{1001}));
  REQUIRE_FALSE(scene.FindEntityByName("Player"));
  REQUIRE(scene.FindEntityByUUID(Rod::UUID{1002}) == enemy);
}

TEST_CASE("Scene selects the primary camera", "[scene]") {
  Rod::Scene scene;
  scene.OnViewportResize(1920, 1080);
  REQUIRE_FALSE(scene.GetPrimaryCameraEntity());

  auto minimap = scene.CreateEntity("Minimap");
  minimap.AddComponent<Rod::CameraComponent>().Primary = false;
  REQUIRE_FALSE(scene.GetPrimaryCameraEntity());

  auto camera = scene.CreateEntity("Camera");
  camera.AddComponent<Rod::CameraComponent>();
  REQUIRE(scene.GetPrimaryCameraEntity() == camera);

  scene.DestroyEntity(camera);
  REQUIRE_FALSE(scene.GetPrimaryCameraEntity());
}

TEST_CASE("Scene copy keeps deep copied components", "[scene]") {
  Rod::Scene scene;
  auto player = scene.CreateEntityWithUUID(Rod::UUID{1001}, "Player");
  player.GetComponent<Rod::TransformComponent>().Translation = {1.0f, 2.0f,
                                                                3.0f};
  player.AddComponent<Rod::SpriteRendererComponent>().TilingFactor = 2.0f;

  Rod::Scene copy(scene);
  auto copiedPlayer = copy.FindEntityByUUID(player.GetUUID());

  REQUIRE(copiedPlayer);
  REQUIRE(copiedPlayer.GetName() == "Player");
  REQUIRE(copiedPlayer.HasComponent<Rod::TransformComponent>());
  REQUIRE(copiedPlayer.HasComponent<Rod::SpriteRendererComponent>());
  REQUIRE(copiedPlayer.GetComponent<Rod::TransformComponent>().Translation ==
          glm::vec3(1.0f, 2.0f, 3.0f));
  REQUIRE(
      copiedPlayer.GetComponent<Rod::SpriteRendererComponent>().TilingFactor ==
      2.0f);

  copiedPlayer.GetComponent<Rod::TransformComponent>().Translation.x = 10.0f;
  copiedPlayer.GetComponent<Rod::SpriteRendererComponent>().TilingFactor = 5.0f;
  REQUIRE(player.GetComponent<Rod::TransformComponent>().Translation.x == 1.0f);
  REQUIRE(player.GetComponent<Rod::SpriteRendererComponent>().TilingFactor ==
          2.0f);

  copy.DestroyEntity(copiedPlayer);
  REQUIRE(scene.FindEntityByUUID(Rod::UUID{1001}) == player);
}
