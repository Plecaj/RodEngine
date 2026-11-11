#include "rdpch.h"

#include "SceneHierarchyPanel.h"

#include "Rod/Scene/Components.h"
#include "Rod/Scripting/ScriptEngine.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_internal.h>

#include <stdexcept>
#include <typeinfo> // IWYU pragma: keep

namespace Rod {

	SceneHierarchyPanel::SceneHierarchyPanel(const std::filesystem::path& assetsPath) : m_AssetsPath(assetsPath)
	{}

	void SceneHierarchyPanel::SetContext(Scene* scene)
	{
		m_Context = scene;
		m_SelectionContext = {};
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		DrawSceneHierarchy();
		DrawPropertiesPanel();
	}

	void SceneHierarchyPanel::DrawSceneHierarchy()
	{
		ImGui::Begin("Scene Hierarchy");
		if (!m_Context)
		{
			ImGui::End();
			return;
		}

		for (auto entityID : m_Context->m_Registry.storage<entt::entity>())
		{
			const Entity entity{entityID, m_Context};
			DrawEntityNode(entity);
		}

		HandleHierarchyBlankSpace();

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawPropertiesPanel()
	{
		ImGui::Begin("Properties");

		if (m_SelectionContext)
			DrawComponents(m_SelectionContext);

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		if (!entity)
			return;

		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags =
		    ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		const bool opened = ImGui::TreeNodeEx(NodeID(static_cast<uint32_t>(entity)), flags, "%s", tag.c_str());

		if (ImGui::IsItemClicked())
			m_SelectionContext = entity;

		const bool entityDeleted = HandleEntityContextMenu();

		if (opened)
		{
			const ImGuiTreeNodeFlags childFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			if (ImGui::TreeNodeEx(NodeID(ENTITY_CHILD_NODE_ID), childFlags, "%s", tag.c_str()))
				ImGui::TreePop();
			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
		}
	}

	void SceneHierarchyPanel::HandleHierarchyBlankSpace()
	{
		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectionContext = {};

		if (ImGui::BeginPopupContextWindow(nullptr, 1 | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
				m_Context->CreateEntity("Empty Entity");
			ImGui::EndPopup();
		}
	}

	bool SceneHierarchyPanel::HandleEntityContextMenu()
	{
		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem(nullptr, 1 | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;
			ImGui::EndPopup();
		}
		return entityDeleted;
	}

	const void* SceneHierarchyPanel::NodeID(uintptr_t value)
	{
		// ImGui hashes this opaque identifier without dereferencing it.
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
		return reinterpret_cast<const void*>(value);
	}

	bool SceneHierarchyPanel::DrawMoreOptionsButton(const char* id, const ImVec2& size)
	{
		constexpr float half = 0.5f;
		constexpr float rounding = 3.0f;
		constexpr float spacing = 4.0f;
		constexpr float radius = 1.6f;
		ImGui::InvisibleButton(id, size);
		const bool hovered = ImGui::IsItemHovered();
		const bool active = ImGui::IsItemActive();
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const ImVec2 min = ImGui::GetItemRectMin();
		const ImVec2 max = ImGui::GetItemRectMax();
		const ImU32 background = active    ? IM_COL32(40, 45, 53, 255)
		                         : hovered ? IM_COL32(50, 56, 66, 255)
		                                   : IM_COL32(0, 0, 0, 0);
		constexpr ImU32 dotColor = IM_COL32(220, 225, 232, 255);
		drawList->AddRectFilled(min, max, background, rounding);
		const ImVec2 center{(min.x + max.x) * half, (min.y + max.y) * half};
		drawList->AddCircleFilled({center.x - spacing, center.y}, radius, dotColor);
		drawList->AddCircleFilled(center, radius, dotColor);
		drawList->AddCircleFilled({center.x + spacing, center.y}, radius, dotColor);
		return ImGui::IsItemClicked(ImGuiMouseButton_Left);
	}

	void SceneHierarchyPanel::DrawAxisControl(
	    const AxisStyle& style,
	    float& value,
	    const ImVec2& size,
	    ImFont* font,
	    const Vec3ControlSettings& settings)
	{
		constexpr int styleColorCount = 3;
		ImGui::PushStyleColor(ImGuiCol_Button, style.Normal);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.Hovered);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.Normal);
		ImGui::PushFont(font);
		if (ImGui::Button(style.Label, size))
			value = settings.ResetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(styleColorCount);
		ImGui::SameLine();
		ImGui::DragFloat(style.InputID, &value, settings.Speed, settings.MinBound, settings.MaxBound, "%.2f");
		ImGui::PopItemWidth();
	}

	void SceneHierarchyPanel::DrawVec3Control(const std::string& label, glm::vec3& values)
	{
		DrawVec3Control(label, values, Vec3ControlSettings{});
	}

	void SceneHierarchyPanel::DrawVec3Control(
	    const std::string& label,
	    glm::vec3& values,
	    const Vec3ControlSettings& settings)
	{
		constexpr int columnCount = 2;
		constexpr float linePaddingFactor = 2.0f;
		constexpr float buttonWidthOffset = 3.0f;
		constexpr auto axes = std::to_array<AxisStyle>(
		    {{"X", "##X", {0.8f, 0.1f, 0.15f, 1.0f}, {0.9f, 0.2f, 0.2f, 1.0f}},
		     {"Y", "##Y", {0.2f, 0.7f, 0.2f, 1.0f}, {0.3f, 0.8f, 0.3f, 1.0f}},
		     {"Z", "##Z", {0.1f, 0.25f, 0.8f, 1.0f}, {0.2f, 0.35f, 0.9f, 1.0f}}});
		auto* boldFont = ImGui::GetIO().Fonts->Fonts[0];
		ImGui::PushID(label.c_str());
		ImGui::Columns(columnCount);
		ImGui::SetColumnWidth(0, settings.ColumnWidth);
		ImGui::TextUnformatted(label.c_str());
		ImGui::NextColumn();
		ImGui::PushMultiItemsWidths(static_cast<int>(axes.size()), ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});
		const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * linePaddingFactor;
		const ImVec2 buttonSize{lineHeight + buttonWidthOffset, lineHeight};
		for (size_t axis = 0; axis < axes.size(); ++axis)
		{
			if (axis != 0)
				ImGui::SameLine();
			DrawAxisControl(
			    axes[axis],
			    values[static_cast<glm::vec3::length_type>(axis)],
			    buttonSize,
			    boldFont,
			    settings);
		}
		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
	}

	template <typename T, typename UIFunction>
	void SceneHierarchyPanel::DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
	{
		if (!entity.HasComponent<T>())
			return;
		constexpr ImVec2 framePadding{4.0f, 4.0f};
		constexpr float linePaddingFactor = 2.0f;
		constexpr float settingsButtonOffset = 0.6f;
		constexpr float verticalSpacing = 4.0f;
		constexpr ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
		                                     ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
		                                     ImGuiTreeNodeFlags_FramePadding;
		auto& component = entity.GetComponent<T>();
		const ImVec2 contentRegion = ImGui::GetContentRegionAvail();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, framePadding);
		const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * linePaddingFactor;
		ImGui::Separator();
		const auto typeHash = typeid(T).hash_code();
		const bool open = ImGui::TreeNodeEx(NodeID(typeHash), flags, "%s", name.c_str());
		ImGui::PopStyleVar();
		ImGui::SameLine(contentRegion.x - lineHeight * settingsButtonOffset);
		const std::string popupID = "ComponentSettings" + std::to_string(typeHash);
		const std::string buttonID = "##ComponentSettings" + std::to_string(typeHash);
		if (DrawMoreOptionsButton(buttonID.c_str(), {lineHeight, lineHeight}))
			ImGui::OpenPopup(popupID.c_str());
		bool removeComponent = false;
		if (ImGui::BeginPopup(popupID.c_str()))
		{
			removeComponent = ImGui::MenuItem("Remove Component");
			ImGui::EndPopup();
		}
		if (open)
		{
			ImGui::Dummy({0.0f, verticalSpacing});
			uiFunction(component);
			ImGui::Dummy({0.0f, verticalSpacing});
			ImGui::TreePop();
		}
		if (removeComponent)
			entity.RemoveComponent<T>();
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		DrawTag(entity);
		DrawAddComponentButton(entity);

		DrawTransformComponent(entity);
		DrawCameraComponent(entity);
		DrawSpriteRendererComponent(entity);
		DrawMeshComponent(entity);
		DrawDirectionalLightComponent(entity);
		DrawScriptComponent(entity);
	}

	void SceneHierarchyPanel::DrawTag(Entity entity)
	{
		if (!entity.HasComponent<TagComponent>())
			return;

		auto& tag = entity.GetComponent<TagComponent>().Tag;

		std::array<char, TEXT_BUFFER_SIZE> buffer{};
		tag.copy(buffer.data(), buffer.size() - 1);

		constexpr float addComponentWidth = ADD_COMPONENT_WIDTH;
		const float availableWidth = ImGui::GetContentRegionAvail().x;
		ImGui::PushItemWidth(ImMax(MIN_TAG_WIDTH, availableWidth - addComponentWidth - ITEM_SPACING));
		if (ImGui::InputText("##", buffer.data(), buffer.size()))
			tag = std::string(buffer.data());
		ImGui::PopItemWidth();
	}

	void SceneHierarchyPanel::DrawAddComponentButton(Entity entity)
	{
		ImGui::SameLine(0.0f, ITEM_SPACING);
		if (ImGui::Button("Add Component", ImVec2(ADD_COMPONENT_WIDTH, 0.0f)))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent"))
		{
			if (ImGui::MenuItem("Camera") && !entity.HasComponent<CameraComponent>())
			{
				entity.AddComponent<CameraComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Sprite Renderer") && !entity.HasComponent<SpriteRendererComponent>())
			{
				entity.AddComponent<SpriteRendererComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Mesh") && !entity.HasComponent<MeshComponent>())
			{
				entity.AddComponent<MeshComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Directional Light") && !entity.HasComponent<DirectionalLightComponent>())
			{
				entity.AddComponent<DirectionalLightComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Script") && !entity.HasComponent<ScriptComponent>())
			{
				entity.AddComponent<ScriptComponent>();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		ImGui::Dummy(ImVec2(0.0f, COMPONENT_BOTTOM_SPACING));
	}

	void SceneHierarchyPanel::DrawTransformComponent(Entity entity)
	{
		DrawComponent<TransformComponent>("Transform", entity, [](auto& component) {
			DrawVec3Control("Translation", component.Translation);

			glm::vec3 rotation = glm::degrees(component.Rotation);
			DrawVec3Control("Rotation", rotation);
			component.Rotation = glm::radians(rotation);

			DrawVec3Control("Scale", component.Scale, Vec3ControlSettings{.Speed = 1.0f});
		});
	}

	void SceneHierarchyPanel::DrawProjectionSelector(SceneCamera& camera)
	{
		constexpr auto names = std::to_array<const char*>({"Perspective", "Orthographic"});
		const auto projectionIndex = static_cast<size_t>(camera.GetProjectionType());
		const char* current = names[std::min(projectionIndex, names.size() - 1)];
		if (ImGui::BeginCombo("Projection", current))
		{
			for (size_t index = 0; index < names.size(); ++index)
			{
				const bool selected = current == names[index];
				if (ImGui::Selectable(names[index], selected))
				{
					current = names[index];
					camera.SetProjectionType(static_cast<SceneCamera::ProjectionType>(index));
				}
				if (selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}

	void SceneHierarchyPanel::DrawPerspectiveControls(SceneCamera& camera)
	{
		float fov = glm::degrees(camera.GetPerspectiveVerticalFOV());
		if (ImGui::DragFloat("Vertical FOV", &fov))
			camera.SetPerspectiveVerticalFOV(glm::radians(fov));
		float nearClip = camera.GetPerspectiveNearClip();
		if (ImGui::DragFloat("Near", &nearClip))
			camera.SetPerspectiveNearClip(nearClip);
		float farClip = camera.GetPerspectiveFarClip();
		if (ImGui::DragFloat("Far", &farClip))
			camera.SetPerspectiveFarClip(farClip);
	}

	void SceneHierarchyPanel::DrawOrthographicControls(CameraComponent& component)
	{
		auto& camera = component.Camera;
		float size = camera.GetOrthographicSize();
		if (ImGui::DragFloat("Size", &size))
			camera.SetOrthographicSize(size);
		float nearClip = camera.GetOrthographicNearClip();
		if (ImGui::DragFloat("Near", &nearClip))
			camera.SetOrthographicNearClip(nearClip);
		float farClip = camera.GetOrthographicFarClip();
		if (ImGui::DragFloat("Far", &farClip))
			camera.SetOrthographicFarClip(farClip);
		ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
	}

	void SceneHierarchyPanel::DrawCameraComponent(Entity entity)
	{
		DrawComponent<CameraComponent>("Camera", entity, [](auto& component) {
			ImGui::Checkbox("Primary", &component.Primary);
			DrawProjectionSelector(component.Camera);
			if (component.Camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				DrawPerspectiveControls(component.Camera);
			if (component.Camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				DrawOrthographicControls(component);
		});
	}

	void SceneHierarchyPanel::DrawSpriteRendererComponent(Entity entity)
	{
		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [this](auto& component) {
			ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));

			if (ImGui::Button("Texture", ImVec2(ASSET_BUTTON_WIDTH, 0.0f)))
			{
				component.Texture = nullptr;
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_TEXTURE_ITEM"))
				{
					const char* path = static_cast<const char*>(payload->Data);
					const std::filesystem::path texturePath = m_AssetsPath / path;
					component.Texture = Texture2D::Create(texturePath.string());
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::DragFloat("Tiling Factor", &component.TilingFactor, DEFAULT_DRAG_SPEED, 0.0f, MAX_TILING_FACTOR);
		});
	}

	void SceneHierarchyPanel::DrawMeshComponent(Entity entity)
	{
		DrawComponent<MeshComponent>("Mesh", entity, [this](auto& component) {
			const std::string meshPath = component.Mesh ? component.Mesh->GetPath() : "None";
			ImGui::Text("Asset: %s", meshPath.c_str());

			if (ImGui::Button("Mesh", ImVec2(ASSET_BUTTON_WIDTH, 0.0f)))
			{
				component.SetMesh(nullptr);
				return;
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_MESH_ITEM"))
				{
					const char* path = static_cast<const char*>(payload->Data);
					const std::filesystem::path meshAssetPath = m_AssetsPath / path;
					try
					{
						component.SetMesh(CreateRef<Mesh>(meshAssetPath.string()));
					}
					catch (const std::runtime_error& exception)
					{
						RD_CORE_ERROR("{}", exception.what());
					}
				}
				ImGui::EndDragDropTarget();
			}
			if (!component.Mesh)
				return;

			auto& material = component.Material;
			glm::vec4 albedo = material.GetAlbedo();
			glm::vec3 emissive = material.GetEmissive();
			float roughness = material.GetRoughness();
			float metallic = material.GetMetallic();

			ImGui::ColorEdit4("Albedo", glm::value_ptr(albedo));
			ImGui::ColorEdit3("Emissive", glm::value_ptr(emissive));
			ImGui::DragFloat("Roughness", &roughness, FINE_DRAG_SPEED, 0.0f, 1.0f);
			ImGui::DragFloat("Metallic", &metallic, FINE_DRAG_SPEED, 0.0f, 1.0f);

			material.SetAlbedo(albedo);
			material.SetEmissive(emissive);
			material.SetRoughness(roughness);
			material.SetMetallic(metallic);
		});
	}

	void SceneHierarchyPanel::DrawDirectionalLightComponent(Entity entity)
	{
		DrawComponent<DirectionalLightComponent>("Directional Light", entity, [](auto& component) {
			DrawVec3Control(
			    "Direction",
			    component.Direction,
			    Vec3ControlSettings{.Speed = FINE_DRAG_SPEED, .MinBound = -1.0f, .MaxBound = 1.0f});
			component.Direction = glm::normalize(component.Direction);
			ImGui::ColorEdit3("Color", glm::value_ptr(component.Color));
			ImGui::DragFloat("Intensity", &component.Intensity, FINE_DRAG_SPEED, 0.0f, 1.0f);
		});
	}

	template <typename Vector>
	std::string SceneHierarchyPanel::VecToFieldValue(const Vector& value)
	{
		std::string result = std::to_string(value[0]);
		for (typename Vector::length_type index = 1; index < value.length(); ++index)
			result += "," + std::to_string(value[index]);
		return result;
	}

	template <typename Vector>
	Vector SceneHierarchyPanel::FieldValueToVector(const std::string& value)
	{
		Vector result(0.0f);
		std::istringstream stream(value);
		for (typename Vector::length_type index = 0; index < result.length(); ++index)
		{
			char separator = '\0';
			if (index != 0 && (!(stream >> separator) || separator != ','))
				return Vector(0.0f);
			if (!(stream >> result[index]))
				return Vector(0.0f);
		}
		return result;
	}

	void SceneHierarchyPanel::SyncScriptFields(ScriptComponent& component)
	{
		ScriptClass* scriptClass = ScriptEngine::GetScriptClass(component.ClassName);
		if (!scriptClass)
			return;

		ScriptFieldMap currentFields = component.Fields;
		component.Fields.clear();

		for (const auto& [name, field] : scriptClass->GetFields())
		{
			auto existingField = currentFields.find(name);
			component.Fields[name] = field;
			if (existingField != currentFields.end() && !existingField->second.Value.empty())
				component.Fields[name].Value = existingField->second.Value;
		}
	}

	void SceneHierarchyPanel::PushRuntimeScriptField(Entity entity, const std::string& fieldName)
	{
		if (ScriptEngine::IsRuntimeRunning())
			ScriptEngine::SetRuntimeFieldValue(entity, fieldName);
	}

	void SceneHierarchyPanel::DrawScriptClassSelector(ScriptComponent& component)
	{
		const auto& classes = ScriptEngine::GetScriptClasses();
		const char* currentClass = component.ClassName.empty() ? "None" : component.ClassName.c_str();
		if (ImGui::BeginCombo("Class", currentClass))
		{
			for (const auto& [name, scriptClass] : classes)
			{
				const bool selected = component.ClassName == name;
				if (ImGui::Selectable(name.c_str(), selected))
				{
					component.ClassName = name;
					SyncScriptFields(component);
				}
				if (selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		if (!component.ClassName.empty())
			SyncScriptFields(component);
	}

	bool SceneHierarchyPanel::DrawFloatingScriptField(const std::string& name, ScriptFieldInstance& field)
	{
		float value = 0.0f;
		if (!field.Value.empty())
			value = field.Field.Type == ScriptFieldType::Double ? static_cast<float>(std::stod(field.Value))
			                                                    : std::stof(field.Value);
		if (!ImGui::DragFloat(name.c_str(), &value, DEFAULT_DRAG_SPEED))
			return false;
		field.Value = std::to_string(value);
		return true;
	}

	bool SceneHierarchyPanel::DrawBooleanScriptField(const std::string& name, ScriptFieldInstance& field)
	{
		bool value = field.Value == "true" || field.Value == "1";
		if (!ImGui::Checkbox(name.c_str(), &value))
			return false;
		field.Value = value ? "true" : "false";
		return true;
	}

	bool SceneHierarchyPanel::DrawIntegerScriptField(const std::string& name, ScriptFieldInstance& field)
	{
		int value = field.Value.empty() ? 0 : std::stoi(field.Value);
		if (!ImGui::DragInt(name.c_str(), &value))
			return false;
		field.Value = std::to_string(value);
		return true;
	}

	bool SceneHierarchyPanel::DrawStringScriptField(const std::string& name, ScriptFieldInstance& field)
	{
		std::array<char, TEXT_BUFFER_SIZE> buffer{};
		field.Value.copy(buffer.data(), buffer.size() - 1);
		if (!ImGui::InputText(name.c_str(), buffer.data(), buffer.size()))
			return false;
		field.Value = buffer.data();
		return true;
	}

	template <typename Vector>
	bool SceneHierarchyPanel::DrawVectorScriptField(const std::string& name, ScriptFieldInstance& field)
	{
		auto value = FieldValueToVector<Vector>(field.Value);
		if (!ImGui::DragScalarN(
		        name.c_str(),
		        ImGuiDataType_Float,
		        glm::value_ptr(value),
		        static_cast<int>(value.length()),
		        DEFAULT_DRAG_SPEED))
			return false;
		field.Value = VecToFieldValue(value);
		return true;
	}

	bool SceneHierarchyPanel::DrawScriptField(const std::string& name, ScriptFieldInstance& field)
	{
		const auto type = field.Field.Type;
		switch (type)
		{
			case ScriptFieldType::Char:
			case ScriptFieldType::Byte:
			case ScriptFieldType::Short:
			case ScriptFieldType::Int:
			case ScriptFieldType::Long:
			case ScriptFieldType::UByte:
			case ScriptFieldType::UShort:
			case ScriptFieldType::UInt:
			case ScriptFieldType::ULong:
			case ScriptFieldType::Entity:
				return DrawIntegerScriptField(name, field);
			case ScriptFieldType::Float:
			case ScriptFieldType::Double:
				return DrawFloatingScriptField(name, field);
			case ScriptFieldType::Bool:
				return DrawBooleanScriptField(name, field);
			case ScriptFieldType::Vector2:
				return DrawVectorScriptField<glm::vec2>(name, field);
			case ScriptFieldType::Vector3:
				return DrawVectorScriptField<glm::vec3>(name, field);
			case ScriptFieldType::Vector4:
				return DrawVectorScriptField<glm::vec4>(name, field);
			case ScriptFieldType::String:
				return DrawStringScriptField(name, field);
			case ScriptFieldType::None:
			default:
				ImGui::Text("%s: unsupported type '%s'", name.c_str(), field.Field.TypeName.c_str());
				return false;
		}
	}

	void SceneHierarchyPanel::DrawScriptComponent(Entity entity)
	{
		DrawComponent<ScriptComponent>("Script", entity, [entity](auto& component) {
			DrawScriptClassSelector(component);
			for (auto& [name, field] : component.Fields)
			{
				if (DrawScriptField(name, field))
					PushRuntimeScriptField(entity, name);
			}
		});
	}

}
