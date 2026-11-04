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
		Entity() = default;
		Entity(entt::entity handle, Scene* scene) noexcept;

		template <typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			RD_CORE_ASSERT(!HasComponent<T>(), "Entity already has component");
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template <typename T>
		[[nodiscard]] T& GetComponent()
		{
			RD_CORE_ASSERT(HasComponent<T>(), "Entity does not have component");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template <typename T>
		[[nodiscard]] const T& GetComponent() const
		{
			RD_CORE_ASSERT(HasComponent<T>(), "Entity does not have component");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template <typename T>
		[[nodiscard]] bool HasComponent() const
		{
			return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
		}

		template <typename T>
		void RemoveComponent()
		{
			RD_CORE_ASSERT(HasComponent<T>(), "Entity does not have component");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		[[nodiscard]] explicit operator bool() const
		{
			return m_EntityHandle != entt::null && m_Scene != nullptr && m_Scene->m_Registry.valid(m_EntityHandle);
		}
		[[nodiscard]] explicit operator uint32_t() const noexcept { return static_cast<uint32_t>(m_EntityHandle); }
		[[nodiscard]] explicit operator entt::entity() const noexcept { return m_EntityHandle; }

		[[nodiscard]] UUID GetUUID() const;
		[[nodiscard]] const std::string& GetName() const;

		[[nodiscard]] bool operator==(const Entity& other) const noexcept
		{
			return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
		}

		[[nodiscard]] bool operator!=(const Entity& other) const noexcept { return !(*this == other); }

	private:
		entt::entity m_EntityHandle = entt::null;
		Scene* m_Scene = nullptr;
	};

}
