#include "rdpch.h"

#include "../TemporaryDirectory.h"
#include "Rod/Scene/Components.h"
#include "Rod/Scene/Entity.h"
#include "Rod/Scene/Scene.h"
#include "Rod/Scene/SceneSerializer.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/yaml.h> // IWYU pragma: keep

#include <fstream>
#include <stdexcept>

namespace {
	class PathOnlyTexture : public Rod::Texture2D
	{
	public:
		explicit PathOnlyTexture(std::string path) : m_Path(std::move(path)) {}
		[[nodiscard]] auto GetWidth() const noexcept -> uint32_t override { return 1; }
		[[nodiscard]] auto GetHeight() const noexcept -> uint32_t override { return 1; }
		[[nodiscard]] auto GetRendererID() const noexcept -> uint32_t override { return 0; }
		void SetData(std::span<const std::byte>) override {}
		[[nodiscard]] auto GetPath() const -> std::string override { return m_Path; }
		void Bind(uint32_t) const override {}
		[[nodiscard]] auto operator==(const Rod::Texture& other) const noexcept -> bool override
		{
			return this == &other;
		}

	private:
		std::string m_Path;
	};

	class SerializationFixture
	{
	public:
		SerializationFixture()
		{
			if (!Rod::Log::GetCoreLogger())
				Rod::Log::Init();
		}

	protected:
		static constexpr uint64_t s_BaseID = 1001;
		static constexpr uint64_t s_PlayerID = 1002;
		static constexpr uint64_t s_CameraID = 1003;
		static constexpr uint64_t s_ExistingID = 42;
		static constexpr glm::uvec2 s_ViewportSize{1920, 1080};
		static constexpr double s_Tolerance = 0.00001;

		[[nodiscard]] auto Filepath() const -> std::string { return (m_Directory.GetPath() / "scene.rod").string(); }
		void WriteScene(const std::string& text) const
		{
			std::ofstream file(Filepath());
			REQUIRE(file.is_open());
			file << text;
			REQUIRE(file.good());
		}
		template <glm::length_t Size>
		static void RequireVectorNear(const glm::vec<Size, float>& actual, const glm::vec<Size, float>& expected)
		{
			for (glm::length_t index = 0; index < Size; ++index)
			{
				CAPTURE(index);
				REQUIRE_THAT(actual[index], Catch::Matchers::WithinAbs(expected[index], s_Tolerance));
			}
		}

	private:
		Rod::Tests::TemporaryDirectory m_Directory;
	};

	class RoundTripFixture : public SerializationFixture
	{
	public:
		RoundTripFixture()
		{
			Rod::SceneSerializer(m_Original).SerializeText(Filepath());
			m_Restored.OnViewportResize(s_ViewportSize.x, s_ViewportSize.y);
			Rod::SceneSerializer(m_Restored).DeserializeText(Filepath());
			m_Expected = m_Original.FindEntityByUUID(Rod::UUID{s_PlayerID});
			m_Actual = m_Restored.FindEntityByUUID(m_Expected.GetUUID());
			REQUIRE(m_Actual);
		}

	protected:
		[[nodiscard]] auto OriginalCamera() -> const Rod::CameraComponent& { return CameraFrom(m_Original); }
		[[nodiscard]] auto RestoredCamera() -> const Rod::CameraComponent& { return CameraFrom(m_Restored); }
		void RequireScriptFieldsEqual() const
		{
			const auto& actual = m_Actual.GetComponent<Rod::ScriptComponent>().Fields;
			const auto& expected = m_Expected.GetComponent<Rod::ScriptComponent>().Fields;
			for (const auto& [name, field] : expected)
			{
				CAPTURE(name);
				REQUIRE(actual.contains(name));
				RequireScriptFieldEqual(actual.at(name), field);
			}
		}
		Rod::Scene m_Original = GenerateTestScene();
		Rod::Scene m_Restored;
		Rod::Entity m_Expected;
		Rod::Entity m_Actual;

	private:
		static constexpr glm::vec3 s_Translation{1.25f, -2.5f, 3.75f};
		static constexpr glm::vec3 s_Rotation{-0.2f, 0.3f, 0.7f};
		static constexpr glm::vec3 s_Scale{2.0f, 0.5f, -1.0f};
		static constexpr glm::vec4 s_SpriteColor{0.2f, 0.4f, 0.6f, 0.8f};
		static constexpr float s_SpriteTiling = 2.75f;
		static constexpr glm::vec3 s_LightDirection{-0.25f, -0.5f, 0.75f};
		static constexpr glm::vec3 s_LightColor{0.9f, 0.6f, 0.3f};
		static constexpr float s_LightIntensity = 3.5f;
		static constexpr float s_OrthographicSize = 15.5f;
		static constexpr float s_OrthographicNear = -4.0f;
		static constexpr float s_OrthographicFar = 8.0f;
		static constexpr float s_PerspectiveDegrees = 65.0f;
		static constexpr float s_PerspectiveNear = 0.25f;
		static constexpr float s_PerspectiveFar = 750.0f;

		static auto GenerateTestScene() -> Rod::Scene
		{
			Rod::Scene scene;
			scene.OnViewportResize(s_ViewportSize.x, s_ViewportSize.y);
			scene.CreateEntityWithUUID(Rod::UUID{s_BaseID}, "Entity");
			auto player = scene.CreateEntityWithUUID(Rod::UUID{s_PlayerID}, "Player: \"one\"\n# tag");
			auto& transform = player.GetComponent<Rod::TransformComponent>();
			transform.Translation = s_Translation;
			transform.Rotation = s_Rotation;
			transform.Scale = s_Scale;
			auto& sprite = player.AddComponent<Rod::SpriteRendererComponent>();
			sprite.Color = s_SpriteColor;
			sprite.TilingFactor = s_SpriteTiling;
			// An empty mesh keeps this test independent of a graphics context.
			player.AddComponent<Rod::MeshComponent>();
			auto& light = player.AddComponent<Rod::DirectionalLightComponent>();
			light.Direction = s_LightDirection;
			light.Color = s_LightColor;
			light.Intensity = s_LightIntensity;
			auto& script = player.AddComponent<Rod::ScriptComponent>("Game.Player");
			script.Fields["Speed"] = {
			    .Field = {.Name = "Speed", .TypeName = "System.Single", .Type = Rod::ScriptFieldType::Float},
			    .Value = "3.75"};
			script.Fields["Target"] = {
			    .Field = {.Name = "Target", .TypeName = "Rod.Entity", .Type = Rod::ScriptFieldType::Entity},
			    .Value = std::to_string(s_BaseID)};
			script.Fields["Message"] = {
			    .Field = {.Name = "Message", .TypeName = "System.String", .Type = Rod::ScriptFieldType::String},
			    .Value = "Hello: \"scene\"\n# text"};
			script.Fields["Empty"] = {
			    .Field = {.Name = "Empty", .TypeName = "System.String", .Type = Rod::ScriptFieldType::String},
			    .Value = ""};
			auto cameraEntity = scene.CreateEntityWithUUID(Rod::UUID{s_CameraID}, "Camera");
			auto& camera = cameraEntity.AddComponent<Rod::CameraComponent>();
			camera.Camera.SetOrthographic(s_OrthographicSize, s_OrthographicNear, s_OrthographicFar);
			camera.Camera.SetPerspective(glm::radians(s_PerspectiveDegrees), s_PerspectiveNear, s_PerspectiveFar);
			camera.Primary = false;
			camera.FixedAspectRatio = true;
			return scene;
		}
		static auto CameraFrom(Rod::Scene& scene) -> const Rod::CameraComponent&
		{
			const auto camera = scene.FindEntityByUUID(Rod::UUID{s_CameraID});
			REQUIRE(camera);
			REQUIRE(camera.HasComponent<Rod::CameraComponent>());
			return camera.GetComponent<Rod::CameraComponent>();
		}
		static void
		RequireScriptFieldEqual(const Rod::ScriptFieldInstance& actual, const Rod::ScriptFieldInstance& expected)
		{
			REQUIRE(actual.Field.Name == expected.Field.Name);
			REQUIRE(actual.Field.Type == expected.Field.Type);
			REQUIRE(actual.Field.TypeName == expected.Field.TypeName);
			REQUIRE(actual.Value == expected.Value);
		}
	};
}

TEST_CASE_METHOD(RoundTripFixture, "Scene serialization preserves entity IDs and names", "[scene][serialization]")
{
	for (const uint64_t id : {s_BaseID, s_PlayerID, s_CameraID})
	{
		CAPTURE(id);
		const auto entity = m_Restored.FindEntityByUUID(Rod::UUID{id});
		REQUIRE(entity);
		REQUIRE(entity.GetUUID() == Rod::UUID{id});
		REQUIRE(entity.GetName() == m_Original.FindEntityByUUID(Rod::UUID{id}).GetName());
	}
}

TEST_CASE_METHOD(RoundTripFixture, "Scene serialization preserves default components", "[scene][serialization]")
{
	const auto bare = m_Restored.FindEntityByUUID(Rod::UUID{s_BaseID});
	const auto& transform = bare.GetComponent<Rod::TransformComponent>();
	RequireVectorNear(transform.Translation, glm::vec3(0.0f));
	RequireVectorNear(transform.Rotation, glm::vec3(0.0f));
	RequireVectorNear(transform.Scale, glm::vec3(1.0f));
	REQUIRE(!bare.HasComponent<Rod::CameraComponent>());
	REQUIRE(!bare.HasComponent<Rod::SpriteRendererComponent>());
	REQUIRE(!bare.HasComponent<Rod::MeshComponent>());
	REQUIRE(!bare.HasComponent<Rod::DirectionalLightComponent>());
	REQUIRE(!bare.HasComponent<Rod::ScriptComponent>());
}

TEST_CASE_METHOD(RoundTripFixture, "Scene serialization preserves transforms", "[scene][serialization]")
{
	const auto& before = m_Expected.GetComponent<Rod::TransformComponent>();
	const auto& after = m_Actual.GetComponent<Rod::TransformComponent>();
	RequireVectorNear(after.Translation, before.Translation);
	RequireVectorNear(after.Rotation, before.Rotation);
	RequireVectorNear(after.Scale, before.Scale);
}

TEST_CASE_METHOD(RoundTripFixture, "Scene serialization preserves sprites and null meshes", "[scene][serialization]")
{
	REQUIRE(m_Actual.HasComponent<Rod::SpriteRendererComponent>());
	const auto& before = m_Expected.GetComponent<Rod::SpriteRendererComponent>();
	const auto& after = m_Actual.GetComponent<Rod::SpriteRendererComponent>();
	RequireVectorNear(after.Color, before.Color);
	REQUIRE_THAT(after.TilingFactor, Catch::Matchers::WithinAbs(before.TilingFactor, s_Tolerance));
	REQUIRE(!after.Texture);
	REQUIRE(m_Actual.HasComponent<Rod::MeshComponent>());
	REQUIRE(!m_Actual.GetComponent<Rod::MeshComponent>().Mesh);
}

TEST_CASE_METHOD(RoundTripFixture, "Scene serialization preserves directional lights", "[scene][serialization]")
{
	REQUIRE(m_Actual.HasComponent<Rod::DirectionalLightComponent>());
	const auto& before = m_Expected.GetComponent<Rod::DirectionalLightComponent>();
	const auto& after = m_Actual.GetComponent<Rod::DirectionalLightComponent>();
	RequireVectorNear(after.Direction, before.Direction);
	RequireVectorNear(after.Color, before.Color);
	REQUIRE_THAT(after.Intensity, Catch::Matchers::WithinAbs(before.Intensity, s_Tolerance));
}

TEST_CASE_METHOD(
    RoundTripFixture,
    "Scene serialization preserves camera flags and projection type",
    "[scene][serialization]")
{
	const auto& after = RestoredCamera();
	REQUIRE(after.Camera.GetProjectionType() == Rod::SceneCamera::ProjectionType::Perspective);
	REQUIRE(!after.Primary);
	REQUIRE(after.FixedAspectRatio);
	REQUIRE(!m_Restored.GetPrimaryCameraEntity());
}

TEST_CASE_METHOD(RoundTripFixture, "Scene serialization preserves perspective settings", "[scene][serialization]")
{
	const auto& before = OriginalCamera().Camera;
	const auto& after = RestoredCamera().Camera;
	REQUIRE_THAT(
	    after.GetPerspectiveVerticalFOV(),
	    Catch::Matchers::WithinAbs(before.GetPerspectiveVerticalFOV(), s_Tolerance));
	REQUIRE_THAT(
	    after.GetPerspectiveNearClip(),
	    Catch::Matchers::WithinAbs(before.GetPerspectiveNearClip(), s_Tolerance));
	REQUIRE_THAT(
	    after.GetPerspectiveFarClip(),
	    Catch::Matchers::WithinAbs(before.GetPerspectiveFarClip(), s_Tolerance));
}

TEST_CASE_METHOD(
    RoundTripFixture,
    "Scene serialization preserves inactive orthographic settings",
    "[scene][serialization]")
{
	const auto& before = OriginalCamera().Camera;
	const auto& after = RestoredCamera().Camera;
	REQUIRE_THAT(after.GetOrthographicSize(), Catch::Matchers::WithinAbs(before.GetOrthographicSize(), s_Tolerance));
	REQUIRE_THAT(
	    after.GetOrthographicNearClip(),
	    Catch::Matchers::WithinAbs(before.GetOrthographicNearClip(), s_Tolerance));
	REQUIRE_THAT(
	    after.GetOrthographicFarClip(),
	    Catch::Matchers::WithinAbs(before.GetOrthographicFarClip(), s_Tolerance));
}

TEST_CASE_METHOD(
    RoundTripFixture,
    "Scene serialization preserves script classes and field counts",
    "[scene][serialization]")
{
	REQUIRE(m_Actual.HasComponent<Rod::ScriptComponent>());
	const auto& before = m_Expected.GetComponent<Rod::ScriptComponent>();
	const auto& after = m_Actual.GetComponent<Rod::ScriptComponent>();
	REQUIRE(after.ClassName == before.ClassName);
	REQUIRE(after.Fields.size() == before.Fields.size());
}

TEST_CASE_METHOD(
    RoundTripFixture,
    "Scene serialization preserves script metadata and escaped strings",
    "[scene][serialization]")
{
	REQUIRE(m_Actual.HasComponent<Rod::ScriptComponent>());
	RequireScriptFieldsEqual();
}

TEST_CASE_METHOD(SerializationFixture, "Empty scenes load", "[scene][serialization]")
{
	WriteScene("Scene: Empty\nEntities: []\n");
	Rod::Scene scene;
	Rod::SceneSerializer(scene).DeserializeText(Filepath());
	REQUIRE(scene == Rod::Scene{});
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

	Rod::Scene scene;
	scene.CreateEntityWithUUID(Rod::UUID{s_ExistingID}, "Existing");
	const Rod::Scene original = scene;
	REQUIRE_THROWS_AS(Rod::SceneSerializer(scene).DeserializeText(Filepath()), std::runtime_error);
	REQUIRE(scene == original);
}

TEST_CASE_METHOD(SerializationFixture, "Invalid components roll back scene loading", "[scene][serialization]")
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
	Rod::Scene scene;
	scene.CreateEntityWithUUID(Rod::UUID{s_ExistingID}, "Existing");
	const Rod::Scene original = scene;
	REQUIRE_THROWS_AS(Rod::SceneSerializer(scene).DeserializeText(Filepath()), std::runtime_error);
	REQUIRE(scene == original);
}

TEST_CASE_METHOD(SerializationFixture, "Duplicate entity IDs roll back scene loading", "[scene][serialization]")
{
	WriteScene("Scene: Invalid\nEntities:\n  - Entity: 1001\n  - Entity: 1001\n");
	Rod::Scene scene;
	scene.CreateEntityWithUUID(Rod::UUID{s_ExistingID}, "Existing");
	const Rod::Scene original = scene;
	REQUIRE_THROWS_AS(Rod::SceneSerializer(scene).DeserializeText(Filepath()), std::runtime_error);
	REQUIRE(scene == original);
}

TEST_CASE_METHOD(SerializationFixture, "Scene saves replace existing files", "[scene][serialization]")
{
	WriteScene("Previous scene contents\n");
	Rod::Scene scene;
	scene.CreateEntityWithUUID(Rod::UUID{s_PlayerID}, "Player");
	Rod::SceneSerializer(scene).SerializeText(Filepath());
	Rod::Scene restored;
	Rod::SceneSerializer(restored).DeserializeText(Filepath());
	REQUIRE(restored == scene);
}

TEST_CASE_METHOD(SerializationFixture, "Missing textures roll back scene loading", "[scene][serialization]")
{
	WriteScene(
	    "Scene: Invalid\nEntities:\n  - Entity: 1001\n  - Entity: 1002\n"
	    "    SpriteRendererComponent:\n      Color: [1, 1, 1, 1]\n      TilingFactor: 1\n"
	    "      Texture: " +
	    Filepath() + ".missing.png\n");
	Rod::Scene scene;
	scene.CreateEntityWithUUID(Rod::UUID{s_ExistingID}, "Existing");
	const Rod::Scene original = scene;
	REQUIRE_THROWS_AS(Rod::SceneSerializer(scene).DeserializeText(Filepath()), std::runtime_error);
	REQUIRE(scene == original);
}

TEST_CASE_METHOD(SerializationFixture, "Missing meshes roll back scene loading", "[scene][serialization]")
{
	WriteScene(
	    "Scene: Invalid\nEntities:\n  - Entity: 1001\n  - Entity: 1002\n"
	    "    MeshComponent:\n      Path: " +
	    Filepath() + ".missing.glb\n");
	Rod::Scene scene;
	scene.CreateEntityWithUUID(Rod::UUID{s_ExistingID}, "Existing");
	const Rod::Scene original = scene;
	REQUIRE_THROWS_AS(Rod::SceneSerializer(scene).DeserializeText(Filepath()), std::runtime_error);
	REQUIRE(scene == original);
}

TEST_CASE_METHOD(
    SerializationFixture,
    "Scene saves use project-relative asset paths",
    "[scene][serialization][project]")
{
	const std::filesystem::path projectDirectory = "My Project";
	const auto assetPath = projectDirectory / "Content" / "Textures" / "Player.png";
	Rod::Scene scene;
	auto entity = scene.CreateEntity("Player");
	entity.AddComponent<Rod::SpriteRendererComponent>().Texture = Rod::CreateRef<PathOnlyTexture>(assetPath.string());
	Rod::SceneSerializer(scene, projectDirectory).SerializeText(Filepath());
	const auto data = YAML::LoadFile(Filepath());
	REQUIRE(
	    data["Entities"][0]["SpriteRendererComponent"]["Texture"].as<std::string>() == "Content/Textures/Player.png");
}

TEST_CASE_METHOD(
    SerializationFixture,
    "Scene loads resolve assets against the project directory",
    "[scene][serialization][project]")
{
	const auto projectDirectory = std::filesystem::path(Filepath()).parent_path() / "My Project";
	std::filesystem::path assetPath = "Content/Missing";
	std::string component;
	SECTION("texture")
	{
		assetPath += ".png";
		component = "    SpriteRendererComponent:\n      Color: [1, 1, 1, 1]\n      TilingFactor: 1\n      Texture: ";
	}
	SECTION("mesh")
	{
		assetPath += ".glb";
		component = "    MeshComponent:\n      Path: ";
	}
	WriteScene("Scene: Missing asset\nEntities:\n  - Entity: 1001\n" + component + assetPath.generic_string() + "\n");
	Rod::Scene scene;
	const auto expectedPath = (projectDirectory / assetPath).lexically_normal();
	REQUIRE_THROWS_WITH(
	    Rod::SceneSerializer(scene, projectDirectory).DeserializeText(Filepath()),
	    Catch::Matchers::ContainsSubstring(expectedPath.string()));
	REQUIRE_FALSE(scene.FindEntityByUUID(Rod::UUID{s_BaseID}));
}
