#pragma once

#include "Rod/Core/Core.h"
#include "Rod/Core/UUID.h"
#include "Scene.h"

#include "entt.hpp"

#include <cstdint>
#include <string>
#include <utility>

namespace Rod {

	class Entity
	{
	public:
		constexpr Entity() noexcept = default;
		constexpr Entity(entt::entity handle, Scene& scene) noexcept : m_EntityHandle(handle), m_Scene(&scene) {}

		template <typename T, typename... Args>
		auto AddComponent(Args&&... args) -> T&
		{
			RD_CORE_ASSERT(!HasComponent<T>(), "Entity already has component");
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template <typename T>
		[[nodiscard]] auto GetComponent() -> T&
		{
			RD_CORE_ASSERT(HasComponent<T>(), "Entity does not have component");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template <typename T>
		[[nodiscard]] auto GetComponent() const -> const T&
		{
			RD_CORE_ASSERT(HasComponent<T>(), "Entity does not have component");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template <typename T>
		[[nodiscard]] auto HasComponent() const -> bool
		{
			return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
		}

		template <typename T>
		void RemoveComponent()
		{
			RD_CORE_ASSERT(HasComponent<T>(), "Entity does not have component");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		[[nodiscard]] explicit operator bool() const noexcept
		{
			return m_EntityHandle != entt::null && m_Scene != nullptr && m_Scene->m_Registry.valid(m_EntityHandle);
		}
		[[nodiscard]] constexpr explicit operator uint32_t() const noexcept
		{
			return static_cast<uint32_t>(m_EntityHandle);
		}
		[[nodiscard]] constexpr explicit operator entt::entity() const noexcept { return m_EntityHandle; }

		[[nodiscard]] auto GetUUID() const -> UUID;
		[[nodiscard]] auto GetName() const -> const std::string&;

		[[nodiscard]] constexpr auto operator==(const Entity& other) const noexcept -> bool
		{
			return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
		}

		[[nodiscard]] constexpr auto operator!=(const Entity& other) const noexcept -> bool
		{
			return !(*this == other);
		}

	private:
		entt::entity m_EntityHandle = entt::null;
		ObserverPtr<Scene> m_Scene = nullptr;
	};

}
