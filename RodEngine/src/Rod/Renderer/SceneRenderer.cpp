#include "rdpch.h"

#include "SceneRenderer.h"

#include "Rod/Renderer/Renderer.h"
#include "Rod/Renderer/Renderer2D.h"
#include "Rod/Renderer/Renderer3D.h"
#include "Rod/Scene/Components.h"
#include "Rod/Scene/Scene.h"

namespace Rod {

	struct SceneRenderer::RuntimeCameraData
	{
		Camera* MainCamera = nullptr;
		glm::mat4 Transform = glm::mat4(1.0f);

		[[nodiscard]] bool IsValid() const { return MainCamera != nullptr; }
	};

	SceneRenderer::RuntimeCameraData SceneRenderer::FindRuntimeCamera(Scene& scene)
	{
		auto& registry = scene.m_Registry;
		RuntimeCameraData cameraData;

		auto view = registry.view<CameraComponent, TransformComponent>();
		view.each([&](CameraComponent& camera, TransformComponent& transform) {
			if (camera.Primary)
			{
				cameraData.MainCamera = &camera.Camera;
				cameraData.Transform = transform.GetTransform();
			}
		});

		return cameraData;
	}

	std::vector<DirectionalLightComponent> SceneRenderer::CollectLights(Scene& scene)
	{
		auto& registry = scene.m_Registry;
		std::vector<DirectionalLightComponent> lightSources;

		auto view = registry.view<DirectionalLightComponent>();
		view.each([&](DirectionalLightComponent& light) { lightSources.push_back(light); });

		return lightSources;
	}

	void SceneRenderer::SubmitMeshes(Scene& scene)
	{
		auto& registry = scene.m_Registry;
		auto view = registry.view<TransformComponent, MeshComponent>();
		view.each([&](auto entity, TransformComponent& transform, MeshComponent& mesh) {
			if (!mesh.Mesh)
				return;

			Renderer3D::Submit(mesh.Mesh, transform.GetTransform(), mesh.Material, static_cast<int>(entity));
		});
	}

	void SceneRenderer::SubmitSprites(Scene& scene)
	{
		auto& registry = scene.m_Registry;
		auto view = registry.view<TransformComponent, SpriteRendererComponent>();
		view.each([&](auto entity, TransformComponent& transform, SpriteRendererComponent& sprite) {
			Renderer2D::DrawSprite(transform.GetTransform(), sprite, static_cast<int>(entity));
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
		auto cameraData = FindRuntimeCamera(scene);
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
