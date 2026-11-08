#include "rdpch.h"

#include "Scene.h"

#include "Components.h"
#include "Entity.h"
#include "Rod/Core/Timestep.h"
#include "Rod/Core/UUID.h"
#include "Rod/Renderer/EditorCamera.h"
#include "Rod/Renderer/SceneRenderer.h"
#include "Rod/Scripting/ScriptEngine.h"

namespace Rod {
	namespace {
		template <typename Component, typename Equal>
		bool SameComponent(
		    const entt::registry& left,
		    entt::entity leftEntity,
		    const entt::registry& right,
		    entt::entity rightEntity,
		    Equal equal)
		{
			const bool leftHas = left.all_of<Component>(leftEntity);
			const bool rightHas = right.all_of<Component>(rightEntity);
			return leftHas == rightHas &&
			       (!leftHas || equal(left.get<Component>(leftEntity), right.get<Component>(rightEntity)));
		}

		template <typename Asset>
		bool SameAsset(const Ref<Asset>& left, const Ref<Asset>& right)
		{
			return (!left && !right) || (left && right && left->GetPath() == right->GetPath());
		}

		bool SameScriptFields(const ScriptFieldMap& left, const ScriptFieldMap& right)
		{
			if (left.size() != right.size())
				return false;
			for (const auto& [name, field] : left)
			{
				const auto it = right.find(name);
				if (it == right.end() || field.Field.Name != it->second.Field.Name ||
				    field.Field.TypeName != it->second.Field.TypeName || field.Field.Type != it->second.Field.Type ||
				    field.Value != it->second.Value)
					return false;
			}
			return true;
		}

		bool SameCamera(const SceneCamera& left, const SceneCamera& right)
		{
			if (left.GetProjectionType() != right.GetProjectionType() ||
			    left.GetPerspectiveVerticalFOV() != right.GetPerspectiveVerticalFOV() ||
			    left.GetPerspectiveNearClip() != right.GetPerspectiveNearClip() ||
			    left.GetPerspectiveFarClip() != right.GetPerspectiveFarClip() ||
			    left.GetOrthographicSize() != right.GetOrthographicSize() ||
			    left.GetOrthographicNearClip() != right.GetOrthographicNearClip() ||
			    left.GetOrthographicFarClip() != right.GetOrthographicFarClip())
				return false;
			for (glm::mat4::length_type column = 0; column < left.GetProjection().length(); ++column)
			{
				if (left.GetProjection()[column] != right.GetProjection()[column])
					return false;
			}
			return true;
		}

		bool SameEntityComponents(
		    const entt::registry& left,
		    entt::entity leftEntity,
		    const entt::registry& right,
		    entt::entity rightEntity)
		{
			return SameComponent<TagComponent>(
			           left,
			           leftEntity,
			           right,
			           rightEntity,
			           [](const auto& a, const auto& b) { return a.Tag == b.Tag; }) &&
			       SameComponent<TransformComponent>(
			           left,
			           leftEntity,
			           right,
			           rightEntity,
			           [](const auto& a, const auto& b) {
				           return a.Translation == b.Translation && a.Rotation == b.Rotation && a.Scale == b.Scale;
			           }) &&
			       SameComponent<SpriteRendererComponent>(
			           left,
			           leftEntity,
			           right,
			           rightEntity,
			           [](const auto& a, const auto& b) {
				           return a.Color == b.Color && a.TilingFactor == b.TilingFactor &&
				                  SameAsset(a.Texture, b.Texture);
			           }) &&
			       SameComponent<MeshComponent>(
			           left,
			           leftEntity,
			           right,
			           rightEntity,
			           [](const auto& a, const auto& b) {
				           return SameAsset(a.Mesh, b.Mesh) && a.Material.GetAlbedo() == b.Material.GetAlbedo() &&
				                  a.Material.GetEmissive() == b.Material.GetEmissive() &&
				                  a.Material.GetRoughness() == b.Material.GetRoughness() &&
				                  a.Material.GetMetallic() == b.Material.GetMetallic();
			           }) &&
			       SameComponent<DirectionalLightComponent>(
			           left,
			           leftEntity,
			           right,
			           rightEntity,
			           [](const auto& a, const auto& b) {
				           return a.Direction == b.Direction && a.Color == b.Color && a.Intensity == b.Intensity;
			           }) &&
			       SameComponent<CameraComponent>(
			           left,
			           leftEntity,
			           right,
			           rightEntity,
			           [](const auto& a, const auto& b) {
				           return a.Primary == b.Primary && a.FixedAspectRatio == b.FixedAspectRatio &&
				                  SameCamera(a.Camera, b.Camera);
			           }) &&
			       SameComponent<ScriptComponent>(
			           left,
			           leftEntity,
			           right,
			           rightEntity,
			           [](const auto& a, const auto& b) {
				           return a.ClassName == b.ClassName && SameScriptFields(a.Fields, b.Fields);
			           }) &&
			       SameComponent<NativeScriptComponent>(
			           left,
			           leftEntity,
			           right,
			           rightEntity,
			           [](const auto& a, const auto& b) {
				           return a.InstantiateScript == b.InstantiateScript && a.DestroyScript == b.DestroyScript;
			           });
		}
	}

	template <typename... Components>
	void Scene::CopyComponent(
	    entt::registry& dst,
	    const entt::registry& src,
	    const std::unordered_map<uint64_t, entt::entity>& entityMap)
	{
		(
		    [&]() {
			    auto view = src.view<Components>();
			    for (auto srcEntity : view)
			    {
				    const UUID uuid = src.get<IDComponent>(srcEntity).ID;
				    const entt::entity dstEntity = entityMap.at(static_cast<uint64_t>(uuid));
				    auto& srcComponent = src.get<Components>(srcEntity);
				    dst.emplace_or_replace<Components>(dstEntity, srcComponent);
			    }
		    }(),
		    ...);
	}

	Scene::Scene(const Scene& other) : m_ViewportWidth(other.m_ViewportWidth), m_ViewportHeight(other.m_ViewportHeight)
	{

		std::unordered_map<uint64_t, entt::entity> entityMap;
		auto idView = other.m_Registry.view<IDComponent>();
		for (auto entityID : idView)
		{
			const UUID uuid = other.m_Registry.get<IDComponent>(entityID).ID;
			const auto& name = other.m_Registry.get<TagComponent>(entityID).Tag;
			const Entity newEntity = this->CreateEntityWithUUID(uuid, name);
			entityMap[static_cast<uint64_t>(uuid)] = static_cast<entt::entity>(newEntity);
		}

		CopyComponent<
		    TransformComponent,
		    SpriteRendererComponent,
		    MeshComponent,
		    DirectionalLightComponent,
		    CameraComponent,
		    ScriptComponent>(this->m_Registry, other.m_Registry, entityMap);

		auto nativeScriptView = other.m_Registry.view<NativeScriptComponent, IDComponent>();
		for (auto entityID : nativeScriptView)
		{
			const UUID uuid = other.m_Registry.get<IDComponent>(entityID).ID;
			auto dstEntity = entityMap.at(static_cast<uint64_t>(uuid));
			auto srcComponent = other.m_Registry.get<NativeScriptComponent>(entityID);
			srcComponent.Instance = nullptr;
			this->m_Registry.emplace_or_replace<NativeScriptComponent>(dstEntity, srcComponent);
		}
	}

	Scene& Scene::operator=(const Scene& other)
	{
		if (this != &other)
		{
			Scene copy(other);
			*this = std::move(copy);
		}
		return *this;
	}

	Scene::Scene(Scene&& other) noexcept
	    : m_Registry(std::move(other.m_Registry)), m_ViewportWidth(other.m_ViewportWidth),
	      m_ViewportHeight(other.m_ViewportHeight)
	{
		RebindNativeScripts();
	}

	Scene& Scene::operator=(Scene&& other) noexcept(false)
	{
		if (this != &other)
		{
			DestroyNativeScripts();
			m_Registry = std::move(other.m_Registry);
			m_ViewportWidth = other.m_ViewportWidth;
			m_ViewportHeight = other.m_ViewportHeight;
			RebindNativeScripts();
		}
		return *this;
	}

	void Scene::RebindNativeScripts()
	{
		std::as_const(m_Registry).view<NativeScriptComponent>().each([this](auto entity, const auto& script) {
			if (script.Instance)
				script.Instance->m_Entity = Entity{entity, this};
		});
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity = {m_Registry.create(), this};
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

		m_Registry.destroy(static_cast<entt::entity>(entity));
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

	bool Scene::operator==(const Scene& other) const
	{
		if (this == &other)
			return true;
		if (m_ViewportWidth != other.m_ViewportWidth || m_ViewportHeight != other.m_ViewportHeight)
			return false;

		std::unordered_map<uint64_t, entt::entity> otherEntities;
		for (const auto entity : other.m_Registry.view<entt::entity>())
		{
			if (!other.m_Registry.all_of<IDComponent>(entity))
				return false;
			const auto id = static_cast<uint64_t>(other.m_Registry.get<IDComponent>(entity).ID);
			if (!otherEntities.emplace(id, entity).second)
				return false;
		}

		for (const auto entity : m_Registry.view<entt::entity>())
		{
			if (!m_Registry.all_of<IDComponent>(entity))
				return false;
			const auto id = static_cast<uint64_t>(m_Registry.get<IDComponent>(entity).ID);
			const auto it = otherEntities.find(id);
			if (it == otherEntities.end() || !SameEntityComponents(m_Registry, entity, other.m_Registry, it->second))
				return false;
			otherEntities.erase(it);
		}
		return otherEntities.empty();
	}

	void Scene::DestroyNativeScripts()
	{
		m_Registry.view<NativeScriptComponent>().each([](auto, auto& script) { DestroyNativeScript(script); });
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

	void Scene::UpdateNativeScript(entt::entity entity, NativeScriptComponent& script, Timestep& ts)
	{
		if (!script.Instance)
		{
			RD_CORE_ASSERT(script.InstantiateScript, "Native script is not bound");
			script.Instance = script.InstantiateScript();
			script.Instance->m_Entity = Entity{entity, this};
			script.Instance->OnCreate();
		}
		script.Instance->OnUpdate(ts);
	}

	void Scene::UpdateNativeScripts(Timestep& ts)
	{
		m_Registry.view<NativeScriptComponent>().each(
		    [this, &ts](auto entity, auto& script) { UpdateNativeScript(entity, script, ts); });
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
				return Entity{entity, this};
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
				return Entity{entity, this};
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
				return Entity{entity, this};
		}
		return {};
	}

	template <typename T>
	void Scene::OnComponentAdded(Entity, T&)
	{
		static_assert(false, "Unsupported component type");
	}

	template <>
	void Scene::OnComponentAdded<TransformComponent>(Entity, TransformComponent&)
	{}

	template <>
	void Scene::OnComponentAdded<IDComponent>(Entity, IDComponent&)
	{}

	template <>
	void Scene::OnComponentAdded<CameraComponent>(Entity, CameraComponent& component)
	{
		component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	template <>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity, SpriteRendererComponent&)
	{}

	template <>
	void Scene::OnComponentAdded<MeshComponent>(Entity, MeshComponent&)
	{}

	template <>
	void Scene::OnComponentAdded<DirectionalLightComponent>(Entity, DirectionalLightComponent&)
	{}

	template <>
	void Scene::OnComponentAdded<TagComponent>(Entity, TagComponent&)
	{}

	template <>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity, NativeScriptComponent&)
	{}

	template <>
	void Scene::OnComponentAdded<ScriptComponent>(Entity, ScriptComponent&)
	{}
}
