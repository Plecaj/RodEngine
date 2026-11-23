#include "rdpch.h"

#include "SceneRenderer.h"

#include "Rod/Renderer/Renderer.h"
#include "Rod/Renderer/Renderer2D.h"
#include "Rod/Renderer/Renderer3D.h"
#include "Rod/Scene/Components.h"
#include "Rod/Scene/Scene.h"

#include <cstdio>
#include <exception>

namespace Rod {

	struct SceneRenderer::RuntimeCameraData
	{
		ObserverPtr<Camera> MainCamera = nullptr;
		glm::mat4 Transform = glm::mat4(1.0f);

		[[nodiscard]] auto IsValid() const -> bool { return MainCamera != nullptr; }
	};

	auto SceneRenderer::FindRuntimeCamera(Scene& scene) -> SceneRenderer::RuntimeCameraData
	{
		auto& registry = scene.m_Registry;
		RuntimeCameraData cameraData;

		const auto view = registry.view<CameraComponent, TransformComponent>();
		view.each([&](CameraComponent& camera, TransformComponent& transform) {
			if (camera.Primary)
			{
				cameraData.MainCamera = &camera.Camera;
				cameraData.Transform = transform.GetTransform();
			}
		});

		return cameraData;
	}

	auto SceneRenderer::CollectLights(Scene& scene) -> std::vector<DirectionalLightComponent>
	{
		auto& registry = scene.m_Registry;
		std::vector<DirectionalLightComponent> lightSources;

		const auto view = registry.view<DirectionalLightComponent>();
		view.each([&](DirectionalLightComponent& light) { lightSources.push_back(light); });

		return lightSources;
	}

	void SceneRenderer::SubmitMeshes(Scene& scene)
	{
		auto& registry = scene.m_Registry;
		const auto view = registry.view<TransformComponent, MeshComponent>();
		view.each([&](auto entity, TransformComponent& transform, MeshComponent& mesh) {
			if (!mesh.Mesh)
				return;

			Renderer3D::Submit(mesh.Mesh, transform.GetTransform(), mesh.Material, static_cast<int>(entity));
		});
	}

	void SceneRenderer::SubmitSprites(Scene& scene)
	{
		auto& registry = scene.m_Registry;
		const auto view = registry.view<TransformComponent, SpriteRendererComponent>();
		view.each([&](auto entity, TransformComponent& transform, SpriteRendererComponent& sprite) {
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
		const auto cameraData = FindRuntimeCamera(scene);
		if (!cameraData.IsValid())
			return;

		auto lights = CollectLights(scene);

		Renderer3D::BeginScene(*cameraData.MainCamera, cameraData.Transform, lights);
		SubmitMeshes(scene);
		Renderer3D::EndScene();

		Renderer2D::BeginScene(*cameraData.MainCamera, cameraData.Transform);
		SubmitSprites(scene);
		Renderer2D::EndScene();
	}

	void SceneRenderer::RenderEditor(Scene& scene, EditorCamera& camera)
	{
		auto lights = CollectLights(scene);

		Renderer3D::BeginScene(camera, lights);
		SubmitMeshes(scene);
		Renderer3D::EndScene();

		Renderer2D::BeginScene(camera);
		SubmitSprites(scene);
		Renderer2D::EndScene();
	}

}
