#pragma once

#include "../EditorInteractionState.h"
#include "Rod.h"

#include <imgui.h>

#include <cstddef>
#include <cstdint>

namespace Rod {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel(std::filesystem::path assetsPath, EditorInteractionState& interaction) noexcept;

		void OnImGuiRender(Scene& scene) const;

	private:
		static constexpr uintptr_t s_EntityChildNodeID = 90714394;
		static constexpr size_t s_TextBufferSize = 256;
		static constexpr float s_DefaultDragSpeed = 0.1f;
		static constexpr float s_FineDragSpeed = 0.01f;
		static constexpr float s_DefaultColumnWidth = 100.0f;
		static constexpr float s_AddComponentWidth = 132.0f;
		static constexpr float s_MinTagWidth = 100.0f;
		static constexpr float s_ItemSpacing = 10.0f;
		static constexpr float s_ComponentBottomSpacing = 6.0f;
		static constexpr float s_AssetButtonWidth = 100.0f;
		static constexpr float s_MaxTilingFactor = 100.0f;
		struct Vec3ControlSettings
		{
			float Speed = s_DefaultDragSpeed;
			float MinBound = 0.0f;
			float MaxBound = 0.0f;
			float ResetValue = 0.0f;
			float ColumnWidth = s_DefaultColumnWidth;
		};
		struct AxisStyle
		{
			const char* Label;
			const char* InputID;
			ImVec4 Normal;
			ImVec4 Hovered;
		};
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

		void DrawSceneHierarchy(Scene& scene) const;
		void DrawPropertiesPanel() const;

		void DrawEntityNode(Scene& scene, Entity entity) const;
		void HandleHierarchyBlankSpace(Scene& scene) const;
		[[nodiscard]] static bool HandleEntityContextMenu();

		static void DrawTag(Entity entity);
		static void DrawAddComponentButton(Entity entity);

		static void DrawTransformComponent(Entity entity);
		static void DrawCameraComponent(Entity entity);
		void DrawSpriteRendererComponent(Entity entity) const;
		void DrawMeshComponent(Entity entity) const;
		static void DrawDirectionalLightComponent(Entity entity);
		static void DrawScriptComponent(Entity entity);

		void DrawComponents(Entity entity) const;

	private:
		std::filesystem::path m_AssetsPath;
		EditorInteractionState& m_Interaction;
	};

}
