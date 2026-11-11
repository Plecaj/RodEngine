#pragma once

#include "Rod.h"

#include <imgui.h>

#include <cstddef>
#include <cstdint>

namespace Rod {

	class SceneHierarchyPanel
	{
	public:
		explicit SceneHierarchyPanel(const std::filesystem::path& assetsPath);

		void SetContext(Scene* scene);

		void OnImGuiRender();

		[[nodiscard]] Entity GetSelectedEntity() const { return m_SelectionContext; }
		void SetSelectedEntity(Entity entity) { m_SelectionContext = entity; }

	private:
		static constexpr uintptr_t ENTITY_CHILD_NODE_ID = 90714394;
		static constexpr size_t TEXT_BUFFER_SIZE = 256;
		static constexpr float DEFAULT_DRAG_SPEED = 0.1f;
		static constexpr float FINE_DRAG_SPEED = 0.01f;
		static constexpr float DEFAULT_COLUMN_WIDTH = 100.0f;
		static constexpr float ADD_COMPONENT_WIDTH = 132.0f;
		static constexpr float MIN_TAG_WIDTH = 100.0f;
		static constexpr float ITEM_SPACING = 10.0f;
		static constexpr float COMPONENT_BOTTOM_SPACING = 6.0f;
		static constexpr float ASSET_BUTTON_WIDTH = 100.0f;
		static constexpr float MAX_TILING_FACTOR = 100.0f;
		struct Vec3ControlSettings
		{
			float Speed = DEFAULT_DRAG_SPEED;
			float MinBound = 0.0f;
			float MaxBound = 0.0f;
			float ResetValue = 0.0f;
			float ColumnWidth = DEFAULT_COLUMN_WIDTH;
		};
		struct AxisStyle
		{
			const char* Label;
			const char* InputID;
			ImVec4 Normal;
			ImVec4 Hovered;
		};
		static const void* NodeID(uintptr_t value);
		static bool DrawMoreOptionsButton(const char* id, const ImVec2& size);
		static void DrawAxisControl(
		    const AxisStyle& style,
		    float& value,
		    const ImVec2& size,
		    ImFont* font,
		    const Vec3ControlSettings& settings);
		static void DrawVec3Control(const std::string& label, glm::vec3& values);
		static void DrawVec3Control(const std::string& label, glm::vec3& values, const Vec3ControlSettings& settings);
		template <typename T, typename UIFunction>
		static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction);
		static void DrawProjectionSelector(SceneCamera& camera);
		static void DrawPerspectiveControls(SceneCamera& camera);
		static void DrawOrthographicControls(CameraComponent& component);
		template <typename Vector>
		static std::string VecToFieldValue(const Vector& value);
		template <typename Vector>
		static Vector FieldValueToVector(const std::string& value);
		static void SyncScriptFields(ScriptComponent& component);
		static void PushRuntimeScriptField(Entity entity, const std::string& fieldName);
		static void DrawScriptClassSelector(ScriptComponent& component);
		static bool DrawFloatingScriptField(const std::string& name, ScriptFieldInstance& field);
		static bool DrawBooleanScriptField(const std::string& name, ScriptFieldInstance& field);
		static bool DrawIntegerScriptField(const std::string& name, ScriptFieldInstance& field);
		static bool DrawStringScriptField(const std::string& name, ScriptFieldInstance& field);
		template <typename Vector>
		static bool DrawVectorScriptField(const std::string& name, ScriptFieldInstance& field);
		static bool DrawScriptField(const std::string& name, ScriptFieldInstance& field);

		void DrawSceneHierarchy();
		void DrawPropertiesPanel();

		void DrawEntityNode(Entity entity);
		void HandleHierarchyBlankSpace();
		bool HandleEntityContextMenu();

		void DrawTag(Entity entity);
		void DrawAddComponentButton(Entity entity);

		void DrawTransformComponent(Entity entity);
		void DrawCameraComponent(Entity entity);
		void DrawSpriteRendererComponent(Entity entity);
		void DrawMeshComponent(Entity entity);
		void DrawDirectionalLightComponent(Entity entity);
		void DrawScriptComponent(Entity entity);

		void DrawComponents(Entity entity);

	private:
		std::filesystem::path m_AssetsPath;
		Scene* m_Context = nullptr;
		Entity m_SelectionContext;
	};

}
