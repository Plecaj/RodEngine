#pragma once

#include "Rod/Core/Timestep.h"
#include "Rod/Core/UUID.h"
#include "Rod/Renderer/EditorCamera.h"

#include "entt.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace Rod {

	class Entity;
	struct NativeScriptComponent;

	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;
		Scene(const Scene& other);
		Scene& operator=(const Scene& other);
		Scene(Scene&& other) noexcept;
		// Replacing a scene invokes user native-script destruction callbacks, which may throw.
		Scene& operator=(Scene&& other) noexcept(false);

		Entity CreateEntity(const std::string& name = "");
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = "");
		void DestroyEntity(Entity entity);

		void OnRuntimeStart();
		void OnRuntimeStop();
		void OnUpdateRuntime(Timestep& ts);
		void OnUpdateEditor(Timestep& ts, EditorCamera& camera);
		void OnViewportResize(uint32_t width, uint32_t height);

		[[nodiscard]] Entity GetPrimaryCameraEntity();
		[[nodiscard]] Entity FindEntityByUUID(UUID uuid);
		[[nodiscard]] Entity FindEntityByName(const std::string& name);

		[[nodiscard]] bool operator==(const Scene& other) const;

		[[nodiscard]] bool IsValidEntityHandle(entt::entity entity) const { return m_Registry.valid(entity); }

	private:
		template <typename... Components>
		static void CopyComponent(
		    entt::registry& dst,
		    const entt::registry& src,
		    const std::unordered_map<uint64_t, entt::entity>& entityMap);

		void DestroyNativeScripts();
		static void DestroyNativeScript(NativeScriptComponent& script);
		void RebindNativeScripts();
		void UpdateNativeScript(entt::entity entity, NativeScriptComponent& script, Timestep& ts);
		void UpdateNativeScripts(Timestep& ts);

		template <typename T>
		void OnComponentAdded(Entity entity, T& component);

	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
		friend class ScriptEngine;
		friend class SceneRenderer;
	};

}
