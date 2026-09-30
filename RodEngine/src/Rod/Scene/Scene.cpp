#include "rdpch.h"
#include "Scene.h"

#include "Rod/Renderer/SceneRenderer.h"
#include "Rod/Scripting/ScriptEngine.h"

#include <glm/glm.hpp>

#include "Entity.h"
#include "Components.h"

namespace Rod {

	Scene::Scene()
	{
	}

	Scene::~Scene() = default;

	template<typename... Components>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<uint64_t, entt::entity>& entityMap)
	{
		([&]()
		{
			auto view = src.view<Components>();
			for (auto srcEntity : view)
			{
				UUID uuid = src.get<IDComponent>(srcEntity).ID;
				entt::entity dstEntity = entityMap.at((uint64_t)uuid);
				auto& srcComponent = src.get<Components>(srcEntity);
				dst.emplace_or_replace<Components>(dstEntity, srcComponent);
			}
		}(), ...);
	}

	Ref<Scene> Scene::Copy(const Ref<Scene>& other)
	{
		Ref<Scene> newScene = CreateRef<Scene>();
		newScene->m_ViewportWidth = other->m_ViewportWidth;
		newScene->m_ViewportHeight = other->m_ViewportHeight;

		std::unordered_map<uint64_t, entt::entity> entityMap;
		auto idView = other->m_Registry.view<IDComponent>();
		for (auto entityID : idView)
		{
			UUID uuid = other->m_Registry.get<IDComponent>(entityID).ID;
			const auto& name = other->m_Registry.get<TagComponent>(entityID).Tag;
			Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
			entityMap[(uint64_t)uuid] = (entt::entity)newEntity;
		}

		CopyComponent<TransformComponent, SpriteRendererComponent, MeshComponent, DirectionalLightComponent,
			CameraComponent, ScriptComponent>(newScene->m_Registry, other->m_Registry, entityMap);

		auto nativeScriptView = other->m_Registry.view<NativeScriptComponent, IDComponent>();
		for (auto entityID : nativeScriptView)
		{
			UUID uuid = other->m_Registry.get<IDComponent>(entityID).ID;
			auto dstEntity = entityMap.at((uint64_t)uuid);
			auto srcComponent = other->m_Registry.get<NativeScriptComponent>(entityID);
			srcComponent.Instance = nullptr;
			newScene->m_Registry.emplace_or_replace<NativeScriptComponent>(dstEntity, srcComponent);
		}

		return newScene;
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		if (entity.HasComponent<NativeScriptComponent>())
			DestroyNativeScript(entity.GetComponent<NativeScriptComponent>());

		m_Registry.destroy(entity);
	}

	void Scene::OnRuntimeStart()
	{
		ScriptEngine::OnRuntimeStart(this);
	}

	void Scene::OnRuntimeStop()
	{
		DestroyNativeScripts();
		ScriptEngine::OnRuntimeStop();
	}

	void Scene::OnUpdateRuntime(Timestep& ts)
	{
		UpdateNativeScripts(ts);
		ScriptEngine::OnRuntimeUpdate(ts);
		SceneRenderer::RenderRuntime(*this);
	}

	void Scene::OnUpdateEditor(Timestep& ts, EditorCamera& camera)
	{
		(void)ts;

		SceneRenderer::RenderEditor(*this, camera);
	}

	void Scene::DestroyNativeScripts()
	{
		m_Registry.view<NativeScriptComponent>().each([this](auto, auto& script)
		{
			DestroyNativeScript(script);
		});
	}

	void Scene::DestroyNativeScript(NativeScriptComponent& script)
	{
		if (!script.Instance)
			return;

		script.Instance->OnDestroy();
		if (script.DestroyScript)
			script.DestroyScript(&script);
		else
			script.Instance = nullptr;
	}

	void Scene::UpdateNativeScripts(Timestep& ts)
	{
		m_Registry.view<NativeScriptComponent>().each([this, &ts](auto entity, auto& script)
		{
			if (!script.Instance)
			{
				RD_CORE_ASSERT(script.InstantiateScript, "Native script is not bound");

				script.Instance = script.InstantiateScript();
				script.Instance->m_Entity = Entity{ entity, this };
				script.Instance->OnCreate();
			}

			script.Instance->OnUpdate(ts);
		});
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (cameraComponent.FixedAspectRatio)
				continue;

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

	Entity Scene::FindEntityByUUID(UUID uuid)
	{
		auto view = m_Registry.view<IDComponent>();
		for (auto entity : view)
		{
			const auto& id = view.get<IDComponent>(entity);
			if (id.ID == uuid)
				return Entity{ entity, this };
		}
		return {};
	}

	Entity Scene::FindEntityByName(const std::string& name)
	{
		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view)
		{
			const auto& tag = view.get<TagComponent>(entity);
			if (tag.Tag == name)
				return Entity{ entity, this };
		}
		return {};
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(false, "Unsupported component type");
	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity, TransformComponent&)
	{
	}

	template<>
	void Scene::OnComponentAdded<IDComponent>(Entity, IDComponent&)
	{
	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity, CameraComponent& component)
	{
		component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity, SpriteRendererComponent&)
	{
	}

	template<>
	void Scene::OnComponentAdded<MeshComponent>(Entity, MeshComponent&)
	{
	}

	template<>
	void Scene::OnComponentAdded<DirectionalLightComponent>(Entity, DirectionalLightComponent&)
	{
	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity, TagComponent&)
	{
	}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity, NativeScriptComponent&)
	{
	}

	template<>
	void Scene::OnComponentAdded<ScriptComponent>(Entity, ScriptComponent&)
	{
	}
}
