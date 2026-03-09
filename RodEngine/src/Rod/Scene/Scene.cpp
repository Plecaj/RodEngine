#include "rdpch.h"
#include "Scene.h"

#include "Rod/Renderer/Renderer2D.h"
#include "Rod/Renderer/Renderer.h"

#include <glm/glm.hpp>

#include "Entity.h"
#include "Components.h"


namespace Rod {

	Scene::Scene()
	{
		
	}

	Scene::~Scene()
	{
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::OnUpdateRuntime(Timestep& ts)
	{
		// Update scripts - TODO: move to Scene::OnScenePlay
		// TODO: also should be calling Instance->OnDetroy on scene stop
		{
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& ncs)
			{
				if (!ncs.Instance)
				{
					ncs.Instance = ncs.InstantiateScript();
					ncs.Instance->m_Entity = Entity{ entity, this };
					ncs.Instance->OnCreate();
				}

				ncs.Instance->OnUpdate(ts);
			});
		}

		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		{
			auto view = m_Registry.view<CameraComponent, TransformComponent>();
			view.each([&](auto entity, CameraComponent& camera, TransformComponent& transform) {
				if (camera.Primary)
				{
					mainCamera = &camera.Camera;
					cameraTransform = transform.GetTransform();
				}
			});
		}

		if (!mainCamera) return;

		std::vector<DirectionalLightComponent> lightSources;

		{
			auto view = m_Registry.view<DirectionalLightComponent>();
			view.each([&](auto entity, DirectionalLightComponent& light) {
				lightSources.push_back(light);
				});
		}

		Renderer::BeginScene(*mainCamera, cameraTransform, lightSources);

		{
			auto view = m_Registry.view<TransformComponent, MeshComponent>();

			view.each([&](auto entity, TransformComponent& transform, MeshComponent& mesh) {
				Renderer::Submit(
					mesh.Mesh->GetVAO(),
					transform.GetTransform()
				);
			});
		}

		Renderer::EndScene();

		Renderer2D::BeginScene(mainCamera->GetProjection(), cameraTransform);
		
		{
			auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
			view.each([&](auto entity, TransformComponent& transform, SpriteRendererComponent& sprite) {
				Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
				});
		}

		Renderer2D::EndScene();
	}

	void Scene::OnUpdateEditor(Timestep& ts, EditorCamera& camera)
	{
		std::vector<DirectionalLightComponent> lightSources;

		{
			auto view = m_Registry.view<DirectionalLightComponent>();
			view.each([&](auto entity, DirectionalLightComponent& light) {
				lightSources.push_back(light);
				});
		}		

		Renderer::BeginScene(camera, lightSources);

		{
			auto view = m_Registry.view<TransformComponent, MeshComponent>();

			view.each([&](auto entity, TransformComponent& transform, MeshComponent& mesh) {
				Renderer::Submit(
					mesh.Mesh->GetVAO(),
					transform.GetTransform()
				);
			});
		}

		Renderer::EndScene();

		/*
		Renderer2D::BeginScene(camera);

		{
			auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
			view.each([&](auto entity, TransformComponent& transform, SpriteRendererComponent& sprite) {
				Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
				});
		}

		Renderer2D::EndScene();
		*/

	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (cameraComponent.FixedAspectRatio) continue;

			cameraComponent.Camera.SetViewportSize(width, height);
		}
		
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity{ entity, this };
		}
		return {};
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(false);
	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<MeshComponent>(Entity entity, MeshComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<DirectionalLightComponent>(Entity entity, DirectionalLightComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{
	}
}