#pragma once

#include "Rod/Core/UUID.h"

#include "entt.hpp"

#include <concepts>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace Rod {

	class Entity;
	struct CameraComponent;
	struct IDComponent;
	struct TagComponent;
	struct TransformComponent;

	template <typename Component>
	concept OptionalComponent = !std::same_as<Component, IDComponent> && !std::same_as<Component, TagComponent> &&
	                            !std::same_as<Component, TransformComponent>;

	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;
		Scene(const Scene& other);
		auto operator=(const Scene& other) -> Scene&;
		Scene(Scene&& other) noexcept = default;
		auto operator=(Scene&& other) noexcept -> Scene&;

		auto CreateEntity(const std::string& name = "") -> Entity;
		auto CreateEntityWithUUID(UUID uuid, const std::string& name = "") -> Entity;
		void DestroyEntity(Entity entity);

		void ResizeViewport(uint32_t width, uint32_t height);
		void SetCameraPrimary(Entity entity, bool primary);

		[[nodiscard]] auto FindPrimaryCamera() -> Entity;
		[[nodiscard]] auto FindEntityByUUID(UUID uuid) -> Entity;
		[[nodiscard]] auto FindEntityByName(const std::string& name) -> Entity;
		[[nodiscard]] auto FindEntityByHandle(entt::entity handle) -> Entity;

		template <typename... Components>
		[[nodiscard]] auto View() const
		{
			return m_Registry.view<Components...>();
		}

		[[nodiscard]] auto operator==(const Scene& other) const -> bool;

	private:
		auto InsertEntity(UUID uuid, std::string_view name) -> Entity;
		[[nodiscard]] auto IsValidEntityHandle(entt::entity entity) const noexcept -> bool
		{
			return m_Registry.valid(entity);
		}

		template <typename Component, typename... Args>
		auto AddComponent(entt::entity handle, Args&&... args) -> Component&
		{
			auto& component = m_Registry.emplace<Component>(handle, std::forward<Args>(args)...);
			if constexpr (std::same_as<Component, CameraComponent>)
				InitializeCamera(handle, component);
			return component;
		}

		template <typename Component>
		[[nodiscard]] auto TryGetComponent(entt::entity handle) noexcept
		{
			if constexpr (std::same_as<Component, IDComponent>)
			{
				return std::as_const(m_Registry).try_get<Component>(handle);
			}
			else
			{
				return m_Registry.try_get<Component>(handle);
			}
		}

		template <typename Component>
		[[nodiscard]] auto TryGetComponent(entt::entity handle) const noexcept
		{
			return m_Registry.try_get<Component>(handle);
		}

		template <OptionalComponent Component>
		void RemoveComponent(entt::entity handle)
		{
			m_Registry.remove<Component>(handle);
		}

		void InitializeCamera(entt::entity handle, CameraComponent& component) const;

	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		friend class Entity;
	};

}
