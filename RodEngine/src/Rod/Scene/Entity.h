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

		template <typename T, typename... Args>
		auto AddComponent(Args&&... args) -> T&
		{
			RD_CORE_ASSERT(!HasComponent<T>(), "Entity already has component");
			return m_Scene->AddComponent<T>(m_EntityHandle, std::forward<Args>(args)...);
		}

		template <typename T>
		[[nodiscard]] auto GetComponent() -> decltype(auto)
		{
			auto* component = TryGetComponent<T>();
			RD_CORE_ASSERT(component, "Entity does not have component");
			return *component;
		}

		template <typename T>
		[[nodiscard]] auto GetComponent() const -> const T&
		{
			const auto* component = TryGetComponent<T>();
			RD_CORE_ASSERT(component, "Entity does not have component");
			return *component;
		}

		template <typename T>
		[[nodiscard]] auto TryGetComponent() noexcept
		{
			return m_Scene ? m_Scene->TryGetComponent<T>(m_EntityHandle) : nullptr;
		}

		template <typename T>
		[[nodiscard]] auto TryGetComponent() const noexcept
		{
			return m_Scene ? std::as_const(*m_Scene).TryGetComponent<T>(m_EntityHandle) : nullptr;
		}

		template <typename T>
		[[nodiscard]] auto HasComponent() const noexcept -> bool
		{
			return TryGetComponent<T>() != nullptr;
		}

		template <OptionalComponent T>
		void RemoveComponent()
		{
			RD_CORE_ASSERT(HasComponent<T>(), "Entity does not have component");
			m_Scene->RemoveComponent<T>(m_EntityHandle);
		}

		[[nodiscard]] explicit operator bool() const noexcept
		{
			return m_Scene != nullptr && m_Scene->IsValidEntityHandle(m_EntityHandle);
		}
		[[nodiscard]] constexpr explicit operator uint32_t() const noexcept
		{
			return static_cast<uint32_t>(m_EntityHandle);
		}
		[[nodiscard]] constexpr explicit operator entt::entity() const noexcept { return m_EntityHandle; }

		[[nodiscard]] auto GetUUID() const -> UUID;
		[[nodiscard]] auto GetName() const -> const std::string&;

		[[nodiscard]] constexpr auto operator==(const Entity& other) const noexcept -> bool = default;

	private:
		constexpr Entity(entt::entity handle, Scene& scene) noexcept : m_EntityHandle(handle), m_Scene(&scene) {}

		entt::entity m_EntityHandle = entt::null;
		ObserverPtr<Scene> m_Scene = nullptr;

		friend class Scene;
	};

}
