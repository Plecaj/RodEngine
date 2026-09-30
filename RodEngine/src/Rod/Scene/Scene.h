#pragma once

#include "Rod/Core/UUID.h"
#include "Rod/Renderer/EditorCamera.h"
#include "Rod/Core/Timestep.h"

#include "entt.hpp"

namespace Rod {

	class Entity;
	struct NativeScriptComponent;

	class Scene
	{
	public:
		Scene();
		~Scene();

		static Ref<Scene> Copy(const Ref<Scene>& other);

		Entity CreateEntity(const std::string& name = "");
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = "");
		void DestroyEntity(Entity entity);

		void OnRuntimeStart();
		void OnRuntimeStop();
		void OnUpdateRuntime(Timestep& ts);
		void OnUpdateEditor(Timestep& ts, EditorCamera& camera);
		void OnViewportResize(uint32_t width, uint32_t height);

		Entity GetPrimaryCameraEntity();
		Entity FindEntityByUUID(UUID uuid);
		Entity FindEntityByName(const std::string& name);
	private:
		void DestroyNativeScripts();
		void DestroyNativeScript(NativeScriptComponent& script);
		void UpdateNativeScripts(Timestep& ts);

		template<typename T>
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
