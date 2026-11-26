#include "rdpch.h"

#include "Scene.h"

#include "Components.h"
#include "Entity.h"
#include "Rod/Core/UUID.h"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>

namespace Rod {
	namespace {
		template <typename Component, typename Equal>
		auto SameComponent(
		    const entt::registry& left,
		    entt::entity leftEntity,
		    const entt::registry& right,
		    entt::entity rightEntity,
		    Equal equal) -> bool
		{
			const bool leftHas = left.all_of<Component>(leftEntity);
			const bool rightHas = right.all_of<Component>(rightEntity);
			return leftHas == rightHas &&
			       (!leftHas || equal(left.get<Component>(leftEntity), right.get<Component>(rightEntity)));
		}

		template <typename Asset>
		auto SameAsset(const Ref<Asset>& left, const Ref<Asset>& right) -> bool
		{
			return (!left && !right) || (left && right && left->GetPath() == right->GetPath());
		}

		auto SameScriptFields(const ScriptFieldMap& left, const ScriptFieldMap& right) -> bool
		{
			if (left.size() != right.size())
				return false;
			return std::ranges::all_of(left, [&right](const auto& entry) {
				const auto& [name, field] = entry;
				const auto otherField = right.find(name);
				return otherField != right.end() && field.Field.Name == otherField->second.Field.Name &&
				       field.Field.TypeName == otherField->second.Field.TypeName &&
				       field.Field.Type == otherField->second.Field.Type && field.Value == otherField->second.Value;
			});
		}

		auto SameCamera(const SceneCamera& left, const SceneCamera& right) -> bool
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

		auto SameEntityComponents(
		    const entt::registry& left,
		    entt::entity leftEntity,
		    const entt::registry& right,
		    entt::entity rightEntity) -> bool
		{
			return SameComponent<TagComponent>(
			           left,
			           leftEntity,
			           right,
			           rightEntity,
			           [](const auto& lhs, const auto& rhs) { return lhs.Tag == rhs.Tag; }) &&
			       SameComponent<TransformComponent>(
			           left,
			           leftEntity,
			           right,
			           rightEntity,
			           [](const auto& lhs, const auto& rhs) {
				           return lhs.Translation == rhs.Translation && lhs.Rotation == rhs.Rotation &&
						          lhs.Scale == rhs.Scale;
			           }) &&
			       SameComponent<SpriteRendererComponent>(
			           left,
			           leftEntity,
			           right,
			           rightEntity,
			           [](const auto& lhs, const auto& rhs) {
				           return lhs.Color == rhs.Color && lhs.TilingFactor == rhs.TilingFactor &&
						          SameAsset(lhs.Texture, rhs.Texture);
			           }) &&
			       SameComponent<MeshComponent>(
			           left,
			           leftEntity,
			           right,
			           rightEntity,
			           [](const auto& lhs, const auto& rhs) {
				           return SameAsset(lhs.Mesh, rhs.Mesh) &&
						          lhs.Material.GetAlbedo() == rhs.Material.GetAlbedo() &&
						          lhs.Material.GetEmissive() == rhs.Material.GetEmissive() &&
						          lhs.Material.GetRoughness() == rhs.Material.GetRoughness() &&
						          lhs.Material.GetMetallic() == rhs.Material.GetMetallic();
			           }) &&
			       SameComponent<DirectionalLightComponent>(
			           left,
			           leftEntity,
			           right,
			           rightEntity,
			           [](const auto& lhs, const auto& rhs) {
				           return lhs.Direction == rhs.Direction && lhs.Color == rhs.Color &&
						          lhs.Intensity == rhs.Intensity;
			           }) &&
			       SameComponent<CameraComponent>(
			           left,
			           leftEntity,
			           right,
			           rightEntity,
			           [](const auto& lhs, const auto& rhs) {
				           return lhs.Primary == rhs.Primary && lhs.FixedAspectRatio == rhs.FixedAspectRatio &&
						          SameCamera(lhs.Camera, rhs.Camera);
			           }) &&
			       SameComponent<ScriptComponent>(
			           left,
			           leftEntity,
			           right,
			           rightEntity,
			           [](const auto& lhs, const auto& rhs) {
				           return lhs.ClassName == rhs.ClassName && SameScriptFields(lhs.Fields, rhs.Fields);
			           });
		}
		template <typename... Components>
		void CopyComponents(
		    const entt::registry& source,
		    entt::entity from,
		    entt::registry& destination,
		    entt::entity destinationEntity)
		{
			const auto copy = [&]<typename Component> {
				if (const auto* component = source.try_get<Component>(from))
					destination.emplace_or_replace<Component>(destinationEntity, *component);
			};
			(copy.template operator()<Components>(), ...);
		}
	}

	Scene::Scene(const Scene& other) : m_ViewportWidth(other.m_ViewportWidth), m_ViewportHeight(other.m_ViewportHeight)
	{
		for (const auto [handle, id, tag] : other.View<IDComponent, TagComponent>().each())
		{
			const auto entity = InsertEntity(id.ID, tag.Tag);
			CopyComponents<
			    TransformComponent,
			    SpriteRendererComponent,
			    MeshComponent,
			    DirectionalLightComponent,
			    CameraComponent,
			    ScriptComponent>(other.m_Registry, handle, m_Registry, static_cast<entt::entity>(entity));
		}
	}

	auto Scene::operator=(const Scene& other) -> Scene&
	{
		if (this != &other)
		{
			Scene copy(other);
			*this = std::move(copy);
		}
		return *this;
	}

	auto Scene::operator=(Scene&& other) noexcept -> Scene&
	{
		if (this != &other)
		{
			m_Registry = std::move(other.m_Registry);
			m_ViewportWidth = other.m_ViewportWidth;
			m_ViewportHeight = other.m_ViewportHeight;
		}
		return *this;
	}

	auto Scene::CreateEntity(const std::string& name) -> Entity
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	auto Scene::CreateEntityWithUUID(UUID uuid, const std::string& name) -> Entity
	{
		if (FindEntityByUUID(uuid))
			throw std::invalid_argument("Duplicate entity UUID: " + std::to_string(static_cast<uint64_t>(uuid)));

		return InsertEntity(uuid, name.empty() ? std::string_view{"Entity"} : std::string_view{name});
	}

	auto Scene::InsertEntity(UUID uuid, std::string_view name) -> Entity
	{
		const auto handle = m_Registry.create();
		try
		{
			m_Registry.emplace<IDComponent>(handle, uuid);
			m_Registry.emplace<TagComponent>(handle, std::string(name));
			m_Registry.emplace<TransformComponent>(handle);
		}
		catch (...)
		{
			m_Registry.destroy(handle);
			throw;
		}
		return {handle, *this};
	}

	void Scene::DestroyEntity(Entity entity)
	{
		if (entity.m_Scene == this && entity)
			m_Registry.destroy(entity.m_EntityHandle);
	}

	auto Scene::FindEntityByHandle(entt::entity handle) -> Entity
	{
		return IsValidEntityHandle(handle) ? Entity{handle, *this} : Entity{};
	}

	auto Scene::operator==(const Scene& other) const -> bool
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
			const auto otherEntity = otherEntities.find(id);
			if (otherEntity == otherEntities.end() ||
			    !SameEntityComponents(m_Registry, entity, other.m_Registry, otherEntity->second))
				return false;
			otherEntities.erase(otherEntity);
		}
		return otherEntities.empty();
	}

	void Scene::ResizeViewport(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		const auto view = m_Registry.view<CameraComponent>();
		for (const auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (cameraComponent.FixedAspectRatio)
				continue;

			cameraComponent.Camera.SetViewportSize(width, height);
		}
	}

	auto Scene::FindPrimaryCamera() -> Entity
	{
		for (const auto [handle, camera] : View<CameraComponent>().each())
		{
			if (camera.Primary)
				return {handle, *this};
		}
		return {};
	}

	void Scene::SetCameraPrimary(Entity entity, bool primary)
	{
		if (entity.m_Scene != this)
			return;
		auto* selected = entity.TryGetComponent<CameraComponent>();
		if (!selected)
			return;

		if (primary)
		{
			for (const auto [handle, camera] : m_Registry.view<CameraComponent>().each())
				camera.Primary = handle == entity.m_EntityHandle;
		}
		else
		{
			selected->Primary = false;
		}
	}

	auto Scene::FindEntityByUUID(UUID uuid) -> Entity
	{
		const auto view = m_Registry.view<IDComponent>();
		for (const auto entity : view)
		{
			const auto& id = view.get<IDComponent>(entity);
			if (id.ID == uuid)
				return Entity{entity, *this};
		}
		return {};
	}

	auto Scene::FindEntityByName(const std::string& name) -> Entity
	{
		const auto view = m_Registry.view<TagComponent>();
		for (const auto entity : view)
		{
			const auto& tag = view.get<TagComponent>(entity);
			if (tag.Tag == name)
				return Entity{entity, *this};
		}
		return {};
	}

	void Scene::InitializeCamera(entt::entity handle, CameraComponent& component) const
	{
		component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
		for (const auto [otherHandle, camera] : View<CameraComponent>().each())
		{
			if (otherHandle != handle && camera.Primary)
			{
				component.Primary = false;
				break;
			}
		}
	}
}
