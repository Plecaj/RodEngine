#include "rdpch.h"
#include "Scene.h"

#include "Rod/Renderer/Renderer2D.h"

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

	void Scene::OnUpdate(Timestep& ts)
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

		// Render 2D
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		{
			auto group = m_Registry.group<CameraComponent>(entt::get<TransformComponent>);
			for (auto entity : group) {
				auto [transform, camera] = group.get<TransformComponent, CameraComponent>(entity);

				if (camera.Priamry)
				{
					mainCamera = &camera.Camera;
					cameraTransform = transform.GetTransform();
					break;
				}
			}
		}

		if (!mainCamera) return;

		Renderer2D::BeginScene(mainCamera->GetProjection(), cameraTransform);

		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for(auto entity : group)
		{
			auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

			Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color);
		}

		Renderer2D::EndScene();
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
}