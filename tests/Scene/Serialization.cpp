#include "rdpch.h"

#include "Rod/Scene/Components.h"
#include "Rod/Scene/Entity.h"
#include "Rod/Scene/Scene.h"
#include "Rod/Scene/SceneSerializer.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
// Keep the umbrella for yaml-cpp node lookup template definitions.
#include <yaml-cpp/yaml.h> // IWYU pragma: keep

#include <fstream>
#include <system_error>

namespace {
	class SerializationFixture
	{
	public:
		SerializationFixture()
		{
			if (!Rod::Log::GetCoreLogger())
				Rod::Log::Init();
			do
			{
				m_Directory = std::filesystem::temp_directory_path() /
				              ("rod-serialization-" + std::to_string(static_cast<uint64_t>(Rod::UUID{})));
			} while (!std::filesystem::create_directory(m_Directory));
		}
		SerializationFixture(const SerializationFixture&) = delete;
		SerializationFixture& operator=(const SerializationFixture&) = delete;
		SerializationFixture(SerializationFixture&&) = delete;
		SerializationFixture& operator=(SerializationFixture&&) = delete;
		~SerializationFixture()
		{
			std::error_code error;
			std::filesystem::remove_all(m_Directory, error);
		}

	protected:
		static constexpr uint64_t BASE_ID = 1001;
		static constexpr uint64_t PLAYER_ID = 1002;
		static constexpr uint64_t CAMERA_ID = 1003;
		static constexpr uint64_t EXISTING_ID = 42;
		static constexpr glm::uvec2 VIEWPORT_SIZE{1920, 1080};
		static constexpr double TOLERANCE = 0.00001;

		[[nodiscard]] std::string Filepath() const { return (m_Directory / "scene.rod").string(); }
		void WriteScene(const std::string& text) const
		{
			std::ofstream file(Filepath());
			REQUIRE(file.is_open());
			file << text;
			file.close();
			REQUIRE(!file.fail());
		}
		template <glm::length_t Size>
		static void RequireVectorNear(const glm::vec<Size, float>& actual, const glm::vec<Size, float>& expected)
		{
			for (glm::length_t index = 0; index < Size; ++index)
			{
				CAPTURE(index);
				REQUIRE_THAT(actual[index], Catch::Matchers::WithinAbs(expected[index], TOLERANCE));
			}
		}

	private:
		std::filesystem::path m_Directory;
	};

	class RoundTripFixture : public SerializationFixture
	{
	public:
		RoundTripFixture()
		{
			Rod::SceneSerializer(m_Original).SerializeText(Filepath());
			m_Restored.OnViewportResize(VIEWPORT_SIZE.x, VIEWPORT_SIZE.y);
			REQUIRE(Rod::SceneSerializer(m_Restored).DeserializeText(Filepath()));
			m_Expected = m_Original.FindEntityByUUID(Rod::UUID{PLAYER_ID});
			m_Actual = m_Restored.FindEntityByUUID(m_Expected.GetUUID());
			REQUIRE(m_Actual);
		}

	protected:
		[[nodiscard]] const Rod::CameraComponent& OriginalCamera() { return CameraFrom(m_Original); }
		[[nodiscard]] const Rod::CameraComponent& RestoredCamera() { return CameraFrom(m_Restored); }
		static void RequireScriptFieldsEqual(const Rod::ScriptFieldMap& actual, const Rod::ScriptFieldMap& expected)
		{
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
		static constexpr glm::vec3 TRANSLATION{1.25f, -2.5f, 3.75f};
		static constexpr glm::vec3 ROTATION{-0.2f, 0.3f, 0.7f};
		static constexpr glm::vec3 SCALE{2.0f, 0.5f, -1.0f};
		static constexpr glm::vec4 SPRITE_COLOR{0.2f, 0.4f, 0.6f, 0.8f};
		static constexpr float SPRITE_TILING = 2.75f;
		static constexpr glm::vec3 LIGHT_DIRECTION{-0.25f, -0.5f, 0.75f};
		static constexpr glm::vec3 LIGHT_COLOR{0.9f, 0.6f, 0.3f};
		static constexpr float LIGHT_INTENSITY = 3.5f;
		static constexpr float ORTHOGRAPHIC_SIZE = 15.5f;
		static constexpr float ORTHOGRAPHIC_NEAR = -4.0f;
		static constexpr float ORTHOGRAPHIC_FAR = 8.0f;
		static constexpr float PERSPECTIVE_DEGREES = 65.0f;
		static constexpr float PERSPECTIVE_NEAR = 0.25f;
		static constexpr float PERSPECTIVE_FAR = 750.0f;

		static Rod::Scene GenerateTestScene()
		{
			Rod::Scene scene;
			scene.OnViewportResize(VIEWPORT_SIZE.x, VIEWPORT_SIZE.y);
			scene.CreateEntityWithUUID(Rod::UUID{BASE_ID}, "Entity");
			auto player = scene.CreateEntityWithUUID(Rod::UUID{PLAYER_ID}, "Player: \"one\"\n# tag");
			auto& transform = player.GetComponent<Rod::TransformComponent>();
			transform.Translation = TRANSLATION;
			transform.Rotation = ROTATION;
			transform.Scale = SCALE;
			auto& sprite = player.AddComponent<Rod::SpriteRendererComponent>();
			sprite.Color = SPRITE_COLOR;
			sprite.TilingFactor = SPRITE_TILING;
			// An empty mesh keeps this test independent of a graphics context.
			player.AddComponent<Rod::MeshComponent>();
			auto& light = player.AddComponent<Rod::DirectionalLightComponent>();
			light.Direction = LIGHT_DIRECTION;
			light.Color = LIGHT_COLOR;
			light.Intensity = LIGHT_INTENSITY;
			auto& script = player.AddComponent<Rod::ScriptComponent>("Game.Player");
			script.Fields["Speed"] = {{"Speed", "System.Single", Rod::ScriptFieldType::Float}, "3.75"};
			script.Fields["Target"] = {{"Target", "Rod.Entity", Rod::ScriptFieldType::Entity}, std::to_string(BASE_ID)};
			script.Fields["Message"] = {
			    {"Message", "System.String", Rod::ScriptFieldType::String},
			    "Hello: \"scene\"\n# text"};
			script.Fields["Empty"] = {{"Empty", "System.String", Rod::ScriptFieldType::String}, ""};
			auto cameraEntity = scene.CreateEntityWithUUID(Rod::UUID{CAMERA_ID}, "Camera");
			auto& camera = cameraEntity.AddComponent<Rod::CameraComponent>();
			camera.Camera.SetOrthographic(ORTHOGRAPHIC_SIZE, ORTHOGRAPHIC_NEAR, ORTHOGRAPHIC_FAR);
			camera.Camera.SetPerspective(glm::radians(PERSPECTIVE_DEGREES), PERSPECTIVE_NEAR, PERSPECTIVE_FAR);
			camera.Primary = false;
			camera.FixedAspectRatio = true;
			return scene;
		}
		static const Rod::CameraComponent& CameraFrom(Rod::Scene& scene)
		{
			const auto camera = scene.FindEntityByUUID(Rod::UUID{CAMERA_ID});
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
	for (const uint64_t id : {BASE_ID, PLAYER_ID, CAMERA_ID})
	{
		CAPTURE(id);
		const auto entity = m_Restored.FindEntityByUUID(Rod::UUID{id});
		REQUIRE(entity);
		REQUIRE(entity.GetUUID() == Rod::UUID{id});
		REQUIRE(entity.GetName() == m_Original.FindEntityByUUID(Rod::UUID{id}).GetName());
		REQUIRE(entity.HasComponent<Rod::TransformComponent>());
	}
}

TEST_CASE_METHOD(RoundTripFixture, "Scene serialization preserves default components", "[scene][serialization]")
{
	const auto bare = m_Restored.FindEntityByUUID(Rod::UUID{BASE_ID});
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
	REQUIRE_THAT(after.TilingFactor, Catch::Matchers::WithinAbs(before.TilingFactor, TOLERANCE));
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
	REQUIRE_THAT(after.Intensity, Catch::Matchers::WithinAbs(before.Intensity, TOLERANCE));
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
	    Catch::Matchers::WithinAbs(before.GetPerspectiveVerticalFOV(), TOLERANCE));
	REQUIRE_THAT(
	    after.GetPerspectiveNearClip(),
	    Catch::Matchers::WithinAbs(before.GetPerspectiveNearClip(), TOLERANCE));
	REQUIRE_THAT(after.GetPerspectiveFarClip(), Catch::Matchers::WithinAbs(before.GetPerspectiveFarClip(), TOLERANCE));
}

TEST_CASE_METHOD(
    RoundTripFixture,
    "Scene serialization preserves inactive orthographic settings",
    "[scene][serialization]")
{
	const auto& before = OriginalCamera().Camera;
	const auto& after = RestoredCamera().Camera;
	REQUIRE_THAT(after.GetOrthographicSize(), Catch::Matchers::WithinAbs(before.GetOrthographicSize(), TOLERANCE));
	REQUIRE_THAT(
	    after.GetOrthographicNearClip(),
	    Catch::Matchers::WithinAbs(before.GetOrthographicNearClip(), TOLERANCE));
	REQUIRE_THAT(
	    after.GetOrthographicFarClip(),
	    Catch::Matchers::WithinAbs(before.GetOrthographicFarClip(), TOLERANCE));
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
	const auto& before = m_Expected.GetComponent<Rod::ScriptComponent>();
	const auto& after = m_Actual.GetComponent<Rod::ScriptComponent>();
	RequireScriptFieldsEqual(after.Fields, before.Fields);
}

TEST_CASE_METHOD(
    SerializationFixture,
    "Scene deserialization accepts a document without entities",
    "[scene][serialization]")
{
	WriteScene("Scene: Empty\n");
	Rod::Scene scene;
	REQUIRE(Rod::SceneSerializer(scene).DeserializeText(Filepath()));
	Rod::SceneSerializer(scene).SerializeText(Filepath());
	REQUIRE(YAML::LoadFile(Filepath())["Entities"].size() == 0);
}

TEST_CASE_METHOD(
    SerializationFixture,
    "Scene deserialization rejects unreadable or invalid documents",
    "[scene][serialization]")
{
	SECTION("missing file")
	{
		REQUIRE(!std::filesystem::exists(Filepath()));
	}
	SECTION("malformed YAML")
	{
		WriteScene("Scene: [unterminated\n");
	}
	SECTION("missing scene header")
	{
		WriteScene("Entities: []\n");
	}
	Rod::Scene scene;
	const auto existing = scene.CreateEntityWithUUID(Rod::UUID{EXISTING_ID}, "Existing");
	REQUIRE(!Rod::SceneSerializer(scene).DeserializeText(Filepath()));
	REQUIRE(scene.FindEntityByUUID(Rod::UUID{EXISTING_ID}) == existing);
	REQUIRE(existing.GetName() == "Existing");
}
