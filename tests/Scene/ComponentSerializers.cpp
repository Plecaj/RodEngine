#include "rdpch.h"

#include "Rod/Scene/Serialization/ComponentSerializers.h"

#include "Rod/Renderer/RenderDevice.h"
#include "Rod/Renderer/Texture.h"
#include "Rod/Scene/Components.h"
#include "Rod/Scripting/ScriptField.h"

#include <catch2/catch_test_macros.hpp>
#include <yaml-cpp/yaml.h> // IWYU pragma: keep

namespace {
	using namespace Rod::SceneSerialization;

	template <typename Serializer>
	auto WriteComponent(const Serializer& serializer, const typename Serializer::Component& component) -> YAML::Node
	{
		YAML::Emitter emitter;
		serializer.Write(emitter, component);
		REQUIRE(emitter.good());
		return YAML::Load(emitter.c_str());
	}

	class PathOnlyTexture : public Rod::Texture2D
	{
	public:
		explicit PathOnlyTexture(std::string path) : m_Path(std::move(path)) {}
		[[nodiscard]] auto GetWidth() const noexcept -> uint32_t override { return 1; }
		[[nodiscard]] auto GetHeight() const noexcept -> uint32_t override { return 1; }
		[[nodiscard]] auto GetRendererID() const noexcept -> uint32_t override { return 0; }
		void SetData(std::span<const std::byte> /*data*/) override {}
		[[nodiscard]] auto GetPath() const -> std::string override { return m_Path; }
		void Bind(uint32_t /*slot*/) const override {}
		[[nodiscard]] auto operator==(const Rod::Texture& other) const noexcept -> bool override
		{
			return this == &other;
		}

	private:
		std::string m_Path;
	};
}

TEST_CASE("Camera serialization preserves both projections and flags", "[scene][serialization][component]")
{
	const auto input = YAML::Load(R"(
Camera:
  ProjectionType: 0
  PerspectiveFOV: 1.25
  PerspectiveNear: 0.25
  PerspectiveFar: 750
  OrthographicSize: 15.5
  OrthographicNear: -4
  OrthographicFar: 8
Primary: false
FixedAspectRatio: true
)");
	const auto requireSettings = [](const Rod::CameraComponent& component) {
		const auto& camera = component.Camera;
		REQUIRE(camera.GetProjectionType() == Rod::SceneCamera::ProjectionType::Perspective);
		REQUIRE(camera.GetPerspectiveVerticalFOV() == 1.25f);
		REQUIRE(camera.GetPerspectiveNearClip() == 0.25f);
		REQUIRE(camera.GetPerspectiveFarClip() == 750.0f);
		REQUIRE(camera.GetOrthographicSize() == 15.5f);
		REQUIRE(camera.GetOrthographicNearClip() == -4.0f);
		REQUIRE(camera.GetOrthographicFarClip() == 8.0f);
		REQUIRE_FALSE(component.Primary);
		REQUIRE(component.FixedAspectRatio);
	};

	Rod::CameraComponent camera;
	CameraSerializer::Read(input, camera);
	requireSettings(camera);

	Rod::CameraComponent restored;
	CameraSerializer::Read(WriteComponent(CameraSerializer{}, camera), restored);
	requireSettings(restored);
}

TEST_CASE("Script serialization preserves field metadata and values", "[scene][serialization][component]")
{
	const auto input = YAML::Load(R"(
ClassName: Game.Player
Fields:
  - {Name: Speed, Type: 1, TypeName: System.Single, Value: "3.75"}
  - {Name: Target, Type: 16, TypeName: Rod.Entity, Value: "1001"}
  - {Name: Message, Type: 17, TypeName: System.String, Value: "Hello: \"scene\"\n# text"}
  - {Name: Empty, Type: 17, TypeName: System.String, Value: ""}
)");
	const auto requireSettings = [](const Rod::ScriptComponent& script) {
		using Rod::ScriptFieldType;
		REQUIRE(script.ClassName == "Game.Player");
		REQUIRE(script.Fields.size() == 4);
		const auto requireField = [&](const char* name, ScriptFieldType type, const char* typeName, const char* value) {
			CAPTURE(name);
			REQUIRE(script.Fields.contains(name));
			const auto& field = script.Fields.at(name);
			REQUIRE(field.Field.Name == name);
			REQUIRE(field.Field.Type == type);
			REQUIRE(field.Field.TypeName == typeName);
			REQUIRE(field.Value == value);
		};
		requireField("Speed", ScriptFieldType::Float, "System.Single", "3.75");
		requireField("Target", ScriptFieldType::Entity, "Rod.Entity", "1001");
		requireField("Message", ScriptFieldType::String, "System.String", "Hello: \"scene\"\n# text");
		requireField("Empty", ScriptFieldType::String, "System.String", "");
	};

	Rod::ScriptComponent script;
	ScriptSerializer::Read(input, script);
	requireSettings(script);

	Rod::ScriptComponent restored;
	ScriptSerializer::Read(WriteComponent(ScriptSerializer{}, script), restored);
	requireSettings(restored);
}

TEST_CASE("Sprite serialization preserves settings and texture references", "[scene][serialization][component]")
{
	const std::filesystem::path projectDirectory = "My Project";
	const AssetPaths paths{projectDirectory};
	const SpriteRendererSerializer serializer{paths};
	Rod::SpriteRendererComponent sprite;
	sprite.Color = {0.25f, 0.5f, 0.75f, 1.0f};
	sprite.TilingFactor = 2.75f;

	const auto withoutTexture = WriteComponent(serializer, sprite);
	REQUIRE(withoutTexture["Texture"].as<std::string>() == "None");
	Rod::SpriteRendererComponent restored;
	serializer.Read(withoutTexture, restored, Rod::RenderDevice{});
	REQUIRE(restored.Color == sprite.Color);
	REQUIRE(restored.TilingFactor == sprite.TilingFactor);
	REQUIRE_FALSE(restored.Texture);

	const auto texturePath = projectDirectory / "Content" / "Textures" / "Player.png";
	sprite.Texture = Rod::CreateRef<PathOnlyTexture>(texturePath.string());
	const auto withTexture = WriteComponent(serializer, sprite);
	REQUIRE(withTexture["Texture"].as<std::string>() == "Content/Textures/Player.png");
}

TEST_CASE("Mesh serialization preserves an unset mesh", "[scene][serialization][component]")
{
	const AssetPaths paths;
	const MeshSerializer serializer{paths};
	const auto output = WriteComponent(serializer, Rod::MeshComponent{});
	REQUIRE(output["Path"].as<std::string>() == "None");

	Rod::MeshComponent restored;
	serializer.Read(output, restored, Rod::RenderDevice{});
	REQUIRE_FALSE(restored.Mesh);
}

TEST_CASE("Directional light serialization preserves its settings", "[scene][serialization][component]")
{
	const Rod::DirectionalLightComponent light{{-0.25f, -0.5f, 0.75f}, {1.0f, 0.5f, 0.25f}, 3.5f};
	Rod::DirectionalLightComponent restored;
	DirectionalLightSerializer::Read(WriteComponent(DirectionalLightSerializer{}, light), restored);
	REQUIRE(restored.Direction == light.Direction);
	REQUIRE(restored.Color == light.Color);
	REQUIRE(restored.Intensity == light.Intensity);
}
