#include "rdpch.h"

#include "SceneRenderer.h"

#include "Rod/Scene/Components.h"
#include "Rod/Scene/Entity.h"
#include "Rod/Scene/Scene.h"

#include <vector>

namespace Rod {
	namespace {
		auto CollectLights(const Scene& scene) -> std::vector<Renderer3D::DirectionalLight>
		{
			std::vector<Renderer3D::DirectionalLight> lights;
			scene.View<DirectionalLightComponent>().each([&](const DirectionalLightComponent& light) {
				lights.push_back({.Direction = light.Direction, .Color = light.Color, .Intensity = light.Intensity});
			});
			return lights;
		}

		void SubmitMeshes(const Scene& scene, Renderer3D& renderer)
		{
			scene.View<TransformComponent, MeshComponent>().each(
			    [&](auto entity, const TransformComponent& transform, const MeshComponent& mesh) {
				    renderer.Submit(mesh.Mesh, transform.GetTransform(), mesh.Material, static_cast<int>(entity));
			    });
		}

		void SubmitSprites(const Scene& scene, Renderer2D& renderer)
		{
			scene.View<TransformComponent, SpriteRendererComponent>().each(
			    [&](auto entity, const TransformComponent& transform, const SpriteRendererComponent& sprite) {
				    renderer.SubmitQuad(
				        transform.GetTransform(),
				        sprite.Texture,
				        sprite.Texture ? sprite.TilingFactor : 1.0f,
				        sprite.Color,
				        static_cast<int>(entity));
			    });
		}
	}

	SceneRenderer::SceneRenderer(const RenderDevice& device, const ShaderLoader& shaders)
	    : m_Meshes(device, shaders), m_Sprites(device, shaders)
	{}

	void SceneRenderer::Render(const Scene& scene, const glm::mat4& viewProjection)
	{
		const auto lights = CollectLights(scene);
		m_Meshes.Begin(viewProjection, lights);
		SubmitMeshes(scene, m_Meshes);
		m_Meshes.End();

		m_Sprites.Begin(viewProjection);
		SubmitSprites(scene, m_Sprites);
		m_Sprites.End();
	}

	void SceneRenderer::RenderRuntime(Scene& scene)
	{
		const auto primary = scene.FindPrimaryCamera();
		if (!primary)
			return;

		const auto& camera = primary.GetComponent<CameraComponent>().Camera;
		const auto transform = primary.GetComponent<TransformComponent>().GetTransform();
		Render(scene, camera.GetProjection() * glm::inverse(transform));
	}

}
