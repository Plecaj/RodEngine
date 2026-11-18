#include "rdpch.h"

#include "../OpenGLFixture.h"
#include "../TemporaryDirectory.h"
#include "Rod/Scene/Components.h"
#include "Rod/Scene/Entity.h"
#include "Rod/Scene/Scene.h"
#include "Rod/Scene/SceneSerializer.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE_METHOD(Rod::Tests::OpenGLFixture, "Scene assets load after relocation", "[scene][serialization][opengl]")
{
	const Rod::Tests::TemporaryDirectory directory;
	const auto project = directory.GetPath() / "Original";
	const auto relocated = directory.GetPath() / "Relocated";
	std::filesystem::create_directories(project / "Assets");
	for (const auto* filename : {"rgba.png", "triangle.glb"})
		std::filesystem::copy_file(AssetPath(filename), project / "Assets" / filename);

	Rod::Scene source;
	auto entity = source.CreateEntity("Player");
	entity.AddComponent<Rod::SpriteRendererComponent>().Texture =
	    Rod::Texture2D::Create((project / "Assets/rgba.png").string());
	auto& mesh = entity.AddComponent<Rod::MeshComponent>();
	mesh.SetMesh(Rod::CreateRef<Rod::Mesh>((project / "Assets/triangle.glb").string()));
	constexpr float roughness = 0.75f;
	mesh.Material.SetRoughness(roughness);
	Rod::SceneSerializer(source, project).SerializeText((project / "scene.rod").string());

	std::filesystem::rename(project, relocated);

	Rod::Scene loaded;
	Rod::SceneSerializer(loaded, relocated).DeserializeText((relocated / "scene.rod").string());
	auto restored = loaded.FindEntityByName("Player");
	REQUIRE(restored);
	const auto& texture = restored.GetComponent<Rod::SpriteRendererComponent>().Texture;
	REQUIRE(texture);
	REQUIRE(std::filesystem::path(texture->GetPath()) == relocated / "Assets" / "rgba.png");
	const auto& restoredMesh = restored.GetComponent<Rod::MeshComponent>();
	REQUIRE(restoredMesh.Mesh);
	REQUIRE(std::filesystem::path(restoredMesh.Mesh->GetPath()) == relocated / "Assets" / "triangle.glb");
	REQUIRE(restoredMesh.Material.GetRoughness() == roughness);
}
