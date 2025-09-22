#pragma once

#include "Rod.h"

namespace Rod {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void SetContext(const Ref<Scene>& scene);

		void OnImGuiRender();

		Entity GetSelectedEntity() const { return m_SelectionContext; }
		void SetSelectedEntity(Entity entity) { m_SelectionContext = entity; }
	private:
		void DrawSceneHierarchy();
		void DrawPropertiesPanel();

		void DrawEntityNode(Entity entity);
		void HandleHierarchyBlankSpace();
		bool HandleEntityContextMenu(Entity entity);

		void DrawTag(Entity entity);
		void DrawAddComponentButton(Entity entity);

		void DrawTransformComponent(Entity entity);
		void DrawCameraComponent(Entity entity);
		void DrawSpriteRendererComponent(Entity entity);

		void DrawComponents(Entity entity);
	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
	};

}