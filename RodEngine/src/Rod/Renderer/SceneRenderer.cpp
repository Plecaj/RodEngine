#include "rdpch.h"

#include "SceneRenderer.h"

#include "Rod/Renderer/Renderer.h"
#include "Rod/Renderer/Renderer2D.h"
#include "Rod/Renderer/Renderer3D.h"
#include "Rod/Scene/Components.h"
#include "Rod/Scene/Entity.h"
#include "Rod/Scene/Scene.h"

#include <cstdio>
#include <exception>

namespace Rod {

	auto SceneRenderer::CollectLights(const Scene& scene) -> std::vector<DirectionalLightComponent>
	{
		std::vector<DirectionalLightComponent> lightSources;

		const auto view = scene.View<DirectionalLightComponent>();
		view.each([&](const DirectionalLightComponent& light) { lightSources.push_back(light); });

		return lightSources;
	}

	void SceneRenderer::SubmitMeshes(const Scene& scene)
	{
		const auto view = scene.View<TransformComponent, MeshComponent>();
		view.each([&](auto entity, const TransformComponent& transform, const MeshComponent& mesh) {
			if (!mesh.Mesh)
				return;

			Renderer3D::Submit(mesh.Mesh, transform.GetTransform(), mesh.Material, static_cast<int>(entity));
		});
	}

	void SceneRenderer::SubmitSprites(const Scene& scene)
	{
		const auto view = scene.View<TransformComponent, SpriteRendererComponent>();
		view.each([&](auto entity, const TransformComponent& transform, const SpriteRendererComponent& sprite) {
			Renderer2D::DrawSprite(transform.GetTransform(), sprite, static_cast<int>(entity));
		});
	}

	SceneRenderer::SceneRenderer(const std::filesystem::path& shaderDirectory)
	{
		Renderer::Init(shaderDirectory);
		try
		{
			Renderer3D::Init();
			Renderer2D::Init();
		}
		catch (...)
		{
			Renderer2D::Shutdown();
			Renderer3D::Shutdown();
			throw;
		}
	}

	SceneRenderer::~SceneRenderer() noexcept
	{
		Renderer2D::Shutdown();
		Renderer3D::Shutdown();
		try
		{
			Renderer::Shutdown();
		}
		catch (const std::exception& error)
		{
			std::fprintf(stderr, "Could not save shader cache: %s\n", error.what());
		}
		catch (...)
		{
			std::fputs("Could not save shader cache\n", stderr);
		}
	}

	void SceneRenderer::RenderRuntime(Scene& scene)
	{
		const auto primary = scene.FindPrimaryCamera();
		if (!primary)
			return;

		const auto& camera = primary.GetComponent<CameraComponent>().Camera;
		const auto transform = primary.GetComponent<TransformComponent>().GetTransform();
		const auto lights = CollectLights(scene);

		Renderer3D::BeginScene(camera, transform, lights);
		SubmitMeshes(scene);
		Renderer3D::EndScene();

		Renderer2D::BeginScene(camera, transform);
		SubmitSprites(scene);
		Renderer2D::EndScene();
	}

	void SceneRenderer::RenderEditor(const Scene& scene, const EditorCamera& camera)
	{
		const auto lights = CollectLights(scene);

		Renderer3D::BeginScene(camera, lights);
		SubmitMeshes(scene);
		Renderer3D::EndScene();

		Renderer2D::BeginScene(camera);
		SubmitSprites(scene);
		Renderer2D::EndScene();
	}

}
