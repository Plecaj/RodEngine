#include "rdpch.h"
#include "SceneRenderer.h"

#include "Rod/Renderer/Renderer.h"
#include "Rod/Renderer/Renderer2D.h"
#include "Rod/Renderer/Renderer3D.h"
#include "Rod/Scene/Components.h"
#include "Rod/Scene/Scene.h"

namespace Rod {

	struct RuntimeCameraData
	{
		Camera* MainCamera = nullptr;
		glm::mat4 Transform = glm::mat4(1.0f);

		bool IsValid() const { return MainCamera != nullptr; }
	};

	static RuntimeCameraData FindRuntimeCamera(entt::registry& registry)
	{
		RuntimeCameraData cameraData;

		auto view = registry.view<CameraComponent, TransformComponent>();
		view.each([&](CameraComponent& camera, TransformComponent& transform)
		{
			if (camera.Primary)
			{
				cameraData.MainCamera = &camera.Camera;
				cameraData.Transform = transform.GetTransform();
			}
		});

		return cameraData;
	}

	static std::vector<DirectionalLightComponent> CollectLights(entt::registry& registry)
	{
		std::vector<DirectionalLightComponent> lightSources;

		auto view = registry.view<DirectionalLightComponent>();
		view.each([&](DirectionalLightComponent& light)
		{
			lightSources.push_back(light);
		});

		return lightSources;
	}

	static void SubmitMeshes(entt::registry& registry)
	{
		auto view = registry.view<TransformComponent, MeshComponent>();
		view.each([&](auto entity, TransformComponent& transform, MeshComponent& mesh)
		{
			if (!mesh.Mesh)
				return;

			Renderer3D::Submit(
				mesh.Mesh->GetVAO(),
				transform.GetTransform(),
				mesh.Mesh->GetMaterial(),
				(int)entity);
		});
	}

	static void SubmitSprites(entt::registry& registry)
	{
		auto view = registry.view<TransformComponent, SpriteRendererComponent>();
		view.each([&](auto entity, TransformComponent& transform, SpriteRendererComponent& sprite)
		{
			Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
		});
	}

	void SceneRenderer::Init()
	{
		RD_PROFILE_FUNCTION();

		Renderer::Init();
		Renderer3D::Init();
		Renderer2D::Init();
	}

	void SceneRenderer::Shutdown()
	{
		RD_PROFILE_FUNCTION();

		Renderer2D::Shutdown();
		Renderer3D::Shutdown();
		Renderer::Shutdown();
	}

	void SceneRenderer::OnViewportResize(uint32_t width, uint32_t height)
	{
		Renderer::OnWindowResize(width, height);
	}

	void SceneRenderer::RenderRuntime(Scene& scene)
	{
		auto& registry = scene.m_Registry;
		auto cameraData = FindRuntimeCamera(registry);
		if (!cameraData.IsValid())
			return;

		auto lights = CollectLights(registry);

		Renderer3D::BeginScene(*cameraData.MainCamera, cameraData.Transform, lights);
		SubmitMeshes(registry);
		Renderer3D::EndScene();

		Renderer2D::BeginScene(*cameraData.MainCamera, cameraData.Transform);
		SubmitSprites(registry);
		Renderer2D::EndScene();
	}

	void SceneRenderer::RenderEditor(Scene& scene, EditorCamera& camera)
	{
		auto& registry = scene.m_Registry;
		auto lights = CollectLights(registry);

		Renderer3D::BeginScene(camera, lights);
		SubmitMeshes(registry);
		Renderer3D::EndScene();

		Renderer2D::BeginScene(camera);
		SubmitSprites(registry);
		Renderer2D::EndScene();
	}

}
