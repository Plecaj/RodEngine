#include "Rod/Scene/Components.h"
#include "Rod/Scene/Entity.h"
#include "Rod/Scene/Scene.h"
#include "Rod/Scene/SceneSerializer.h"
#include "rdpch.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <fstream>
#include <system_error>

namespace {

Rod::Scene GenerateTestScene() {
  Rod::Scene scene;
  scene.OnViewportResize(1920, 1080);
  scene.CreateEntityWithUUID(Rod::UUID{1001}, "Entity");

  auto player =
      scene.CreateEntityWithUUID(Rod::UUID{1002}, "Player: \"one\"\n# tag");
  auto &transform = player.GetComponent<Rod::TransformComponent>();
  transform.Translation = {1.25f, -2.5f, 3.75f};
  transform.Rotation = {-0.2f, 0.3f, 0.7f};
  transform.Scale = {2.0f, 0.5f, -1.0f};

  auto &sprite = player.AddComponent<Rod::SpriteRendererComponent>();
  sprite.Color = {0.2f, 0.4f, 0.6f, 0.8f};
  sprite.TilingFactor = 2.75f;
  player
      .AddComponent<Rod::MeshComponent>(); // It should be empty to keep test
                                           // simple rn and not use graphics
                                           // context, can be reconsidered later

  auto &light = player.AddComponent<Rod::DirectionalLightComponent>();
  light.Direction = {-0.25f, -0.5f, 0.75f};
  light.Color = {0.9f, 0.6f, 0.3f};
  light.Intensity = 3.5f;

  auto &script = player.AddComponent<Rod::ScriptComponent>("Game.Player");
  script.Fields["Speed"] = {
      {"Speed", "System.Single", Rod::ScriptFieldType::Float}, "3.75"};
  script.Fields["Target"] = {
      {"Target", "Rod.Entity", Rod::ScriptFieldType::Entity}, "1001"};
  script.Fields["Message"] = {
      {"Message", "System.String", Rod::ScriptFieldType::String},
      "Hello: \"scene\"\n# text"};
  script.Fields["Empty"] = {
      {"Empty", "System.String", Rod::ScriptFieldType::String}, ""};

  auto cameraEntity = scene.CreateEntityWithUUID(Rod::UUID{1003}, "Camera");
  auto &camera = cameraEntity.AddComponent<Rod::CameraComponent>();
  camera.Camera.SetOrthographic(15.5f, -4.0f, 8.0f);
  camera.Camera.SetPerspective(glm::radians(65.0f), 0.25f, 750.0f);
  camera.Primary = false;
  camera.FixedAspectRatio = true;
  return scene;
}

struct SerializationFixture {
  std::filesystem::path Directory;

  SerializationFixture() {
    if (!Rod::Log::GetCoreLogger())
      Rod::Log::Init();
    do {
      Directory = std::filesystem::temp_directory_path() /
                  ("rod-serialization-" +
                   std::to_string(static_cast<uint64_t>(Rod::UUID{})));
    } while (!std::filesystem::create_directory(Directory));
  }

  SerializationFixture(const SerializationFixture &) = delete;
  SerializationFixture &operator=(const SerializationFixture &) = delete;

  ~SerializationFixture() {
    std::error_code error;
    std::filesystem::remove_all(Directory, error);
  }

  std::string Filepath() const { return (Directory / "scene.rod").string(); }

  void WriteScene(const std::string &text) const {
    std::ofstream file(Filepath());
    REQUIRE(file.is_open());
    file << text;
    file.close();
    REQUIRE_FALSE(file.fail());
  }
};

template <glm::length_t Size>
void RequireVectorNear(const glm::vec<Size, float> &actual,
                       const glm::vec<Size, float> &expected) {
  for (glm::length_t index = 0; index < Size; ++index) {
    CAPTURE(index);
    REQUIRE_THAT(actual[index],
                 Catch::Matchers::WithinAbs(expected[index], 0.00001));
  }
}

} // namespace

TEST_CASE_METHOD(SerializationFixture,
                 "Scene serialization preserves entity IDs and component data",
                 "[scene][serialization]") {
  auto original = GenerateTestScene();
  Rod::SceneSerializer(original).SerializeText(Filepath());
  Rod::Scene restored;
  restored.OnViewportResize(1920, 1080);
  REQUIRE(Rod::SceneSerializer(restored).DeserializeText(Filepath()));

  auto expected = original.FindEntityByUUID(Rod::UUID{1002});
  auto actual = restored.FindEntityByUUID(expected.GetUUID());
  REQUIRE(actual);

  SECTION("IDs, names and default components") {
    for (uint64_t id : {1001, 1002, 1003}) {
      CAPTURE(id);
      auto entity = restored.FindEntityByUUID(Rod::UUID{id});
      REQUIRE(entity);
      REQUIRE(entity.GetUUID() == Rod::UUID{id});
      REQUIRE(entity.GetName() ==
              original.FindEntityByUUID(Rod::UUID{id}).GetName());
      REQUIRE(entity.HasComponent<Rod::TransformComponent>());
    }
    auto bare = restored.FindEntityByUUID(Rod::UUID{1001});
    const auto &transform = bare.GetComponent<Rod::TransformComponent>();
    RequireVectorNear(transform.Translation, glm::vec3(0.0f));
    RequireVectorNear(transform.Rotation, glm::vec3(0.0f));
    RequireVectorNear(transform.Scale, glm::vec3(1.0f));
    REQUIRE_FALSE(bare.HasComponent<Rod::CameraComponent>());
    REQUIRE_FALSE(bare.HasComponent<Rod::SpriteRendererComponent>());
    REQUIRE_FALSE(bare.HasComponent<Rod::MeshComponent>());
    REQUIRE_FALSE(bare.HasComponent<Rod::DirectionalLightComponent>());
    REQUIRE_FALSE(bare.HasComponent<Rod::ScriptComponent>());
  }

  SECTION("transform") {
    const auto &before = expected.GetComponent<Rod::TransformComponent>();
    const auto &after = actual.GetComponent<Rod::TransformComponent>();
    RequireVectorNear(after.Translation, before.Translation);
    RequireVectorNear(after.Rotation, before.Rotation);
    RequireVectorNear(after.Scale, before.Scale);
  }

  SECTION("sprite and null mesh") {
    REQUIRE(actual.HasComponent<Rod::SpriteRendererComponent>());
    const auto &before = expected.GetComponent<Rod::SpriteRendererComponent>();
    const auto &after = actual.GetComponent<Rod::SpriteRendererComponent>();
    RequireVectorNear(after.Color, before.Color);
    REQUIRE_THAT(after.TilingFactor,
                 Catch::Matchers::WithinAbs(before.TilingFactor, 0.00001));
    REQUIRE_FALSE(after.Texture);
    REQUIRE(actual.HasComponent<Rod::MeshComponent>());
    REQUIRE_FALSE(actual.GetComponent<Rod::MeshComponent>().Mesh);
  }

  SECTION("directional light") {
    REQUIRE(actual.HasComponent<Rod::DirectionalLightComponent>());
    const auto &before =
        expected.GetComponent<Rod::DirectionalLightComponent>();
    const auto &after = actual.GetComponent<Rod::DirectionalLightComponent>();
    RequireVectorNear(after.Direction, before.Direction);
    RequireVectorNear(after.Color, before.Color);
    REQUIRE_THAT(after.Intensity,
                 Catch::Matchers::WithinAbs(before.Intensity, 0.00001));
  }

  SECTION("camera settings, including the inactive projection") {
    auto camera = restored.FindEntityByUUID(Rod::UUID{1003});
    REQUIRE(camera);
    REQUIRE(camera.HasComponent<Rod::CameraComponent>());
    const auto &after = camera.GetComponent<Rod::CameraComponent>();
    REQUIRE(after.Camera.GetProjectionType() ==
            Rod::SceneCamera::ProjectionType::Perspective);
    REQUIRE_THAT(after.Camera.GetPerspectiveVerticalFOV(),
                 Catch::Matchers::WithinAbs(glm::radians(65.0f), 0.00001));
    REQUIRE_THAT(after.Camera.GetPerspectiveNearClip(),
                 Catch::Matchers::WithinAbs(0.25f, 0.00001));
    REQUIRE_THAT(after.Camera.GetPerspectiveFarClip(),
                 Catch::Matchers::WithinAbs(750.0f, 0.00001));
    REQUIRE_THAT(after.Camera.GetOrthographicSize(),
                 Catch::Matchers::WithinAbs(15.5f, 0.00001));
    REQUIRE_THAT(after.Camera.GetOrthographicNearClip(),
                 Catch::Matchers::WithinAbs(-4.0f, 0.00001));
    REQUIRE_THAT(after.Camera.GetOrthographicFarClip(),
                 Catch::Matchers::WithinAbs(8.0f, 0.00001));
    REQUIRE_FALSE(after.Primary);
    REQUIRE(after.FixedAspectRatio);
    REQUIRE_FALSE(restored.GetPrimaryCameraEntity());
  }

  SECTION("script metadata, references and escaped strings") {
    REQUIRE(actual.HasComponent<Rod::ScriptComponent>());
    const auto &before = expected.GetComponent<Rod::ScriptComponent>();
    const auto &after = actual.GetComponent<Rod::ScriptComponent>();
    REQUIRE(after.ClassName == before.ClassName);
    REQUIRE(after.Fields.size() == before.Fields.size());
    for (const auto &[name, field] : before.Fields) {
      CAPTURE(name);
      REQUIRE(after.Fields.contains(name));
      const auto &restoredField = after.Fields.at(name);
      REQUIRE(restoredField.Field.Name == field.Field.Name);
      REQUIRE(restoredField.Field.Type == field.Field.Type);
      REQUIRE(restoredField.Field.TypeName == field.Field.TypeName);
      REQUIRE(restoredField.Value == field.Value);
    }
  }
}

TEST_CASE_METHOD(SerializationFixture,
                 "Scene deserialization accepts a document without entities",
                 "[scene][serialization]") {
  WriteScene("Scene: Empty\n");
  Rod::Scene scene;
  REQUIRE(Rod::SceneSerializer(scene).DeserializeText(Filepath()));
  Rod::SceneSerializer(scene).SerializeText(Filepath());
  REQUIRE(YAML::LoadFile(Filepath())["Entities"].size() == 0);
}

TEST_CASE_METHOD(
    SerializationFixture,
    "Scene deserialization rejects unreadable or invalid documents",
    "[scene][serialization]") {
  SECTION("missing file") {
    REQUIRE_FALSE(std::filesystem::exists(Filepath()));
  }
  SECTION("malformed YAML") { WriteScene("Scene: [unterminated\n"); }
  SECTION("missing scene header") { WriteScene("Entities: []\n"); }
  Rod::Scene scene;
  auto existing = scene.CreateEntityWithUUID(Rod::UUID{42}, "Existing");
  REQUIRE_FALSE(Rod::SceneSerializer(scene).DeserializeText(Filepath()));
  REQUIRE(scene.FindEntityByUUID(Rod::UUID{42}) == existing);
  REQUIRE(existing.GetName() == "Existing");
}
