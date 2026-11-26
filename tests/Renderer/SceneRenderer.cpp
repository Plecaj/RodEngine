#include "rdpch.h"

#include "Rod/Renderer/SceneRenderer.h"

#include "../OpenGLFixture.h"
#include "../TemporaryDirectory.h"
#include "Rod/Renderer/FrameBuffer.h"
#include "Rod/Renderer/Mesh.h"
#include "Rod/Renderer/RenderDevice.h"
#include "Rod/Renderer/ShaderLoader.h"
#include "Rod/Scene/Components.h"
#include "Rod/Scene/Entity.h"
#include "Rod/Scene/Scene.h"

#include <catch2/catch_test_macros.hpp>
#include <glad/glad.h>

TEST_CASE_METHOD(
    Rod::Tests::OpenGLFixture,
    "Scene rendering draws a lit mesh and a sprite through the primary camera",
    "[renderer][opengl]")
{
	const Rod::Tests::TemporaryDirectory directory;
	const Rod::RenderDevice device;
	device.ConfigureDefaults();
	const Rod::ShaderLoader loader(ROD_TEST_SHADER_DIRECTORY, directory.GetPath());
	Rod::SceneRenderer renderer(device, loader);

	Rod::Scene scene;
	scene.ResizeViewport(16, 16);
	auto camera = scene.CreateEntity("Main Camera");
	camera.GetComponent<Rod::TransformComponent>().Translation.x = 1.0f;
	camera.AddComponent<Rod::CameraComponent>().Camera.SetOrthographic(2.0f, -1.0f, 1.0f);

	auto triangle = scene.CreateEntity("Triangle");
	triangle.GetComponent<Rod::TransformComponent>().Translation = {0.25f, -0.25f, 0.0f};
	triangle.AddComponent<Rod::MeshComponent>().SetMesh(
	    Rod::CreateRef<Rod::Mesh>(device, AssetPath("triangle.glb").string()));
	scene.CreateEntity("Sun").AddComponent<Rod::DirectionalLightComponent>();

	auto sprite = scene.CreateEntity("Sprite");
	sprite.GetComponent<Rod::TransformComponent>().Translation.x = 1.5f;
	sprite.AddComponent<Rod::SpriteRendererComponent>();

	Rod::FramebufferSpecification spec;
	spec.Width = 16;
	spec.Height = 16;
	spec.Attachments = {
	    Rod::FramebufferTextureSpecification{Rod::FramebufferTextureFormat::RGBA8},
	    Rod::FramebufferTextureSpecification{Rod::FramebufferTextureFormat::RedInteger},
	    Rod::FramebufferTextureSpecification{Rod::FramebufferTextureFormat::DEPTH24},
	};
	const auto target = device.CreateFramebuffer(spec);
	target->Bind();
	target->ClearColorAttachment(0, glm::vec4(0.0f));
	target->ClearColorAttachment(1, -1);
	device.ClearDepth();
	renderer.RenderRuntime(scene);

	REQUIRE(target->ReadPixel(1, 4, 8) == static_cast<int>(static_cast<entt::entity>(triangle)));
	REQUIRE(target->ReadPixel(1, 12, 8) == static_cast<int>(static_cast<entt::entity>(sprite)));
	REQUIRE(glGetError() == GL_NO_ERROR);
}
